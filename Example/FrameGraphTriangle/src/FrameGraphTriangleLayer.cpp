#include "FrameGraphTriangleLayer.h"
#include <GEngine/Graphics/Texture.h>
#include <GEngine/Renderer/RenderGraph.h>
#include <array>
#include <limits>
#include <stdexcept>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <imgui.h>
#include <GEngine/Runtime/ResourcePack.h>
#include <GEngine/Tools/ModelImporter.h>
#include <GEngine/Tools/Serializer.h>
#include <GEngine/Object/Scene/Scene.h>
#include <future>

namespace GEngine
{
	FrameGraphTriangleLayer::FrameGraphTriangleLayer()
		: Layer("FrameGraphTriangle")
	{
	}

	void FrameGraphTriangleLayer::OnImGuiRender()
	{
		ImGui::SetCurrentContext(Application::Get().GetImGuiLayer()->GetContext());
		ImGui::Begin("Dependency upgrade smoke check");
		ImGui::Text("ImGui %s", ImGui::GetVersion());
		ImGui::TextUnformatted("Drag this window outside the main window to check viewports.");
		static int clicks = 0;
		if (ImGui::Button("Check input"))
			++clicks;
		ImGui::Text("Clicks: %d", clicks);
		ImGui::End();
	}

	void FrameGraphTriangleLayer::OnAttach()
	{
        if (std::getenv("GENGINE_EXAMPLE_RESOURCE_CHECK"))
        {
            const auto expected = ReadResource("Assets/ResourceProbe.ppm");
            auto pending = std::async(std::launch::async, [] { return ReadResource("Assets/ResourceProbe.ppm"); });
            const auto loaded = pending.get();
            if (loaded != expected) throw std::runtime_error("Background resource read differs.");
            auto texture = Graphics::GetRenderDevice().CreateTexture2D("Assets/ResourceProbe.ppm");
            if (texture->GetWidth() != 2 || texture->GetHeight() != 2)
                throw std::runtime_error("Archive image decoding/upload failed.");
            ModelImporter model;
            if (!model.LoadMesh("Assets/ResourceProbe.fbx") || !model.GetMesh(0))
                throw std::runtime_error("Archive FBX stream import failed.");
            auto scene = CreateRef<Scene>();
            Serializer::Deserialize("Assets/ResourceProbe.GEScene", scene);
            if (scene->name != "ArchiveProbe") throw std::runtime_error("Archive scene YAML read failed.");
            if (HasResourcePack() && std::filesystem::exists("Assets"))
                throw std::runtime_error("Packaged assets were extracted to disk.");
            GE_INFO("Resource input check passed: owned background bytes, texture upload, archive={} cwd={}",
                HasResourcePack(), std::filesystem::current_path().string());
        }
		const auto capabilities = Graphics::GetCapabilities();
		GE_INFO("Graphics capability snapshot: {} {}.", capabilities.Backend, capabilities.Version);
		for (const auto& source : capabilities.QuerySources)
			GE_INFO("Graphics capability source: {}.", source);

		// This must execute even when assertions are disabled.
		RenderGraph graph;
		int executions = 0;
		graph.AddPass("ReleaseCompileCheck", [&executions]() { ++executions; });
		graph.Execute();
		if (executions != 1)
			throw std::runtime_error("RenderGraph implicit compilation failed.");

		// Declare accesses backwards: inferred ordering must still find the cycle.
		RenderGraph dependencyGraph;
		const auto producer = dependencyGraph.AddPass("Producer", []() {});
		const auto consumer = dependencyGraph.AddPass("Consumer", []() {});
		const auto logical = dependencyGraph.ImportResource("Ordering");
		dependencyGraph.Read(consumer, logical);
		dependencyGraph.Write(producer, logical, RenderGraph::ResourceState::ShaderWrite);
		if (!dependencyGraph.Compile() || !dependencyGraph.Compile())
			throw std::runtime_error("RenderGraph dependency compilation failed.");
		dependencyGraph.AddDependency(producer, consumer);
		GE_INFO("Expected negative test: the next Producer cycle diagnostic is intentional.");
		if (dependencyGraph.Compile())
			throw std::runtime_error("RenderGraph missed an inferred dependency cycle.");

		// Invalid transient graphs must fail before allocating GPU resources.
		for (int scenario = 0; scenario < 2; ++scenario)
		{
			RenderGraph invalid;
			const auto texture = invalid.CreateTransientTexture2D("InvalidTexture", { 16, 16 });
			const auto pass = invalid.AddPass("InvalidAccess", []() {});
			if (scenario == 0) invalid.Read(pass, texture);
			else invalid.Write(pass, texture, RenderGraph::ResourceState::RenderTarget);
			bool rejected = false;
			try { invalid.Compile(); }
			catch (const std::invalid_argument&) { rejected = true; }
			if (!rejected) throw std::runtime_error("RenderGraph accepted an invalid transient access.");
		}
		RenderGraph versions;
		std::vector<int> order;
		auto versionConsumer = versions.BuildPass("VersionConsumer", [&order](FrameContext&) { order.push_back(2); });
		auto versionProducer = versions.BuildPass("VersionProducer", [&order](FrameContext&) { order.push_back(1); });
		auto initial = versions.GetVersion(versions.ImportResource("Versioned"));
		auto written = versionProducer.Write(initial, RenderGraph::ResourceState::ShaderWrite);
		versionConsumer.Read(written);
		versions.Execute();
		if (order != std::vector<int>({ 1, 2 }))
			throw std::runtime_error("Versioned graph ordering failed.");
		versions.Reset();
		bool staleRejected = false;
		try { versions.ReadVersion(0, written); }
		catch (const std::invalid_argument&) { staleRejected = true; }
		if (!staleRejected) throw std::runtime_error("Stale graph version was accepted.");

		RenderGraph overwrite;
		order.clear();
		auto nextWriter = overwrite.BuildPass("Overwrite", [&order](FrameContext&) { order.push_back(3); });
		auto oldReader = overwrite.BuildPass("OldReader", [&order](FrameContext&) { order.push_back(2); });
		auto firstWriter = overwrite.BuildPass("FirstWriter", [&order](FrameContext&) { order.push_back(1); });
		auto base = overwrite.GetVersion(overwrite.ImportResource("SharedStorage"));
		auto firstVersion = firstWriter.Write(base, RenderGraph::ResourceState::ShaderWrite);
		nextWriter.Write(firstVersion, RenderGraph::ResourceState::ShaderWrite);
		oldReader.Read(firstVersion);
		overwrite.Execute();
		if (order != std::vector<int>({ 1, 2, 3 }))
			throw std::runtime_error("Resource overwrite did not wait for old readers.");
		bool branchRejected = false;
		try { firstWriter.Write(base, RenderGraph::ResourceState::ShaderWrite); }
		catch (const std::invalid_argument&) { branchRejected = true; }
		if (!branchRejected) throw std::runtime_error("Branching resource write was accepted.");

		RenderGraph hazards;
		int memoryDependencies = 0;
		hazards.SetTransitionCallback([&memoryDependencies](const FrameContext&, const Ref<GraphicsResource>&,
			RenderGraph::ResourceState before, RenderGraph::ResourceState after)
		{
			if (before == after && after == RenderGraph::ResourceState::ShaderWrite) ++memoryDependencies;
		});
		auto storage = hazards.ImportResource("WriteHazard", RenderGraph::ResourceState::ShaderWrite);
		hazards.Write(hazards.AddPass("WriteA", [] {}), storage, RenderGraph::ResourceState::ShaderWrite);
		hazards.Write(hazards.AddPass("WriteB", [] {}), storage, RenderGraph::ResourceState::ShaderWrite);
		hazards.Execute();
		if (memoryDependencies != 2) throw std::runtime_error("Same-state write dependency was omitted.");

		RenderGraph invalidRangeGraph;
		auto rangeResource = invalidRangeGraph.ImportResource("InvalidRange");
		bool invalidRangeRejected = false;
		try
		{
			invalidRangeGraph.Read(invalidRangeGraph.AddPass("InvalidRange", [] {}), rangeResource,
				RenderGraph::ResourceState::ShaderRead, GraphicsPipelineStage::All,
				{ 0, 0, 0, RenderGraph::SubresourceRange::All });
		}
		catch (const std::invalid_argument&) { invalidRangeRejected = true; }
		if (!invalidRangeRejected) throw std::runtime_error("RenderGraph accepted an empty subresource range.");

		RenderGraph overflowRangeGraph;
		auto overflowResource = overflowRangeGraph.ImportResource("OverflowRange");
		bool overflowRangeRejected = false;
		try
		{
			overflowRangeGraph.Read(overflowRangeGraph.AddPass("OverflowRange", [] {}),
				overflowResource, RenderGraph::ResourceState::ShaderRead, GraphicsPipelineStage::All,
				{ std::numeric_limits<uint32_t>::max(), 1, 0, 1 });
		}
		catch (const std::invalid_argument&) { overflowRangeRejected = true; }
		if (!overflowRangeRejected) throw std::runtime_error("RenderGraph accepted an overflowing subresource range.");

		RenderGraph outOfBoundsRangeGraph;
		auto outOfBoundsTexture = outOfBoundsRangeGraph.CreateTransientTexture2D(
			"OutOfBoundsRange", { 16, 16, 2, 1, RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM });
		outOfBoundsRangeGraph.Read(outOfBoundsRangeGraph.AddPass("OutOfBoundsRange", [] {}),
			outOfBoundsTexture, RenderGraph::ResourceState::ShaderRead, GraphicsPipelineStage::All,
			{ 2, 1, 0, 1 });
		bool outOfBoundsRejected = false;
		try { outOfBoundsRangeGraph.Compile(); }
		catch (const std::invalid_argument&) { outOfBoundsRejected = true; }
		if (!outOfBoundsRejected || outOfBoundsRangeGraph.GetResource(outOfBoundsTexture))
			throw std::runtime_error("RenderGraph allocated a resource before rejecting an out-of-bounds range.");

		RenderGraph unsupportedRangeGraph;
		auto unsupportedResource = unsupportedRangeGraph.ImportResource("UnsupportedRange");
		unsupportedRangeGraph.Read(unsupportedRangeGraph.AddPass("UnsupportedRange", [] {}),
			unsupportedResource, RenderGraph::ResourceState::ShaderRead, GraphicsPipelineStage::All,
			{ 0, 1, 0, 1 });
		bool unsupportedRangeRejected = false;
		try { unsupportedRangeGraph.Compile(); }
		catch (const std::invalid_argument&) { unsupportedRangeRejected = true; }
		if (!unsupportedRangeRejected) throw std::runtime_error("RenderGraph accepted a non-whole range without metadata.");

		RenderGraph allRangeGraph;
		auto allRangeTexture = allRangeGraph.CreateTransientTexture2D(
			"AllRange", { 16, 16, 2, 1, RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM });
		allRangeGraph.Write(allRangeGraph.AddPass("AllRangeWriter", [] {}),
			allRangeTexture, RenderGraph::ResourceState::CopyDestination);
		allRangeGraph.Read(allRangeGraph.AddPass("AllRangeReader", [] {}), allRangeTexture);
		if (!allRangeGraph.Compile())
			throw std::runtime_error("Render graph whole-resource range compilation failed.");
		const auto createdRangeTexture = allRangeGraph.GetTexture2D(allRangeTexture);
		if (!createdRangeTexture || createdRangeTexture->GetMipLevels() != 2)
			throw std::runtime_error("Transient texture mip metadata was not applied.");

		RenderGraph transientArrayGraph;
		const auto transientArray = transientArrayGraph.CreateTransientTexture2DArray(
			"TransientArray", { 16, 16, 2, 1, RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM });
		transientArrayGraph.Write(transientArrayGraph.AddPass("TransientArrayWriter", [] {}),
			transientArray, RenderGraph::ResourceState::CopyDestination);
		transientArrayGraph.Read(transientArrayGraph.AddPass("TransientArrayReader", [] {}),
			transientArray, RenderGraph::ResourceState::ShaderRead, GraphicsPipelineStage::All,
			{ 0, 1, 1, 1 });
		if (!transientArrayGraph.Compile() || !transientArrayGraph.GetResource(transientArray))
			throw std::runtime_error("Transient texture-array metadata was not applied.");

		RenderGraph importedRangeGraph;
		const Ref<Texture> importedTexture = Texture2D::Create(
			16, 16, RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM, 2);
		const auto importedRangeResource = importedRangeGraph.ImportTexture("ImportedRange", importedTexture);
		importedRangeGraph.Read(importedRangeGraph.AddPass("ImportedRange", [] {}),
			importedRangeResource, RenderGraph::ResourceState::ShaderRead, GraphicsPipelineStage::All,
			{ 0, 1, 0, 1 });
		if (!importedRangeGraph.Compile())
			throw std::runtime_error("Imported texture range compilation failed.");

		RenderGraph importedArrayGraph;
		const Ref<Texture> importedArray = Texture2DArray::Create(
			16, 16, 2, RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM);
		const auto importedArrayResource = importedArrayGraph.ImportTexture("ImportedArray", importedArray);
		importedArrayGraph.Read(importedArrayGraph.AddPass("ImportedArray", [] {}),
			importedArrayResource, RenderGraph::ResourceState::ShaderRead, GraphicsPipelineStage::All,
			{ 0, 1, 1, 1 });
		if (!importedArrayGraph.Compile())
			throw std::runtime_error("Imported texture-array layer range compilation failed.");

		RenderGraph rangeTransitionGraph;
		std::vector<std::array<uint32_t, 4>> transitionRanges;
		std::vector<int> rangeOrder;
		rangeTransitionGraph.SetTransitionUsageCallback([&transitionRanges](const FrameContext&,
			const Ref<GraphicsResource>&, const GraphicsResourceUsage&, const GraphicsResourceUsage&,
			const RenderGraph::SubresourceRange& range)
		{
			transitionRanges.push_back({ range.BaseMipLevel, range.MipLevelCount,
				range.BaseArrayLayer, range.ArrayLayerCount });
		});
		const Ref<Texture> rangeTransitionTexture = Texture2D::Create(
			16, 16, RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM, 2);
		const auto rangeTransitionResource = rangeTransitionGraph.ImportTexture(
			"RangeTransition", rangeTransitionTexture, RenderGraph::ResourceState::ShaderRead);
		const auto rangeWriter = rangeTransitionGraph.AddPass("RangeWriter", [&rangeOrder]() { rangeOrder.push_back(1); });
		rangeTransitionGraph.Write(rangeWriter, rangeTransitionResource,
			RenderGraph::ResourceState::CopyDestination, GraphicsPipelineStage::All, { 0, 1, 0, 1 });
		const auto untouchedReader = rangeTransitionGraph.AddPass(
			"UntouchedRangeReader", [&rangeOrder]() { rangeOrder.push_back(2); });
		rangeTransitionGraph.Read(untouchedReader, rangeTransitionResource,
			RenderGraph::ResourceState::ShaderRead, GraphicsPipelineStage::All, { 1, 1, 0, 1 });
		const auto touchedReader = rangeTransitionGraph.AddPass(
			"TouchedRangeReader", [&rangeOrder]() { rangeOrder.push_back(3); });
		rangeTransitionGraph.Read(touchedReader, rangeTransitionResource,
			RenderGraph::ResourceState::ShaderRead, GraphicsPipelineStage::All, { 0, 1, 0, 1 });
		rangeTransitionGraph.Execute();
		const std::vector<std::array<uint32_t, 4>> expectedTransitionRanges = {
			{ 0, 1, 0, 1 }, { 0, 1, 0, 1 }
		};
		if (rangeOrder != std::vector<int>({ 1, 2, 3 }) || transitionRanges != expectedTransitionRanges)
			throw std::runtime_error("RenderGraph did not preserve subresource transition ranges.");

		RenderGraph conservativeRangeGraph;
		const Ref<Texture> conservativeRangeTexture = Texture2D::Create(
			16, 16, RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM, 2);
		const auto conservativeRangeResource = conservativeRangeGraph.ImportTexture(
			"ConservativeRange", conservativeRangeTexture, RenderGraph::ResourceState::Undefined);
		const auto disjointWriter = conservativeRangeGraph.AddPass("DisjointRangeWriter", []() {});
		conservativeRangeGraph.Write(disjointWriter, conservativeRangeResource,
			RenderGraph::ResourceState::ShaderWrite, GraphicsPipelineStage::All, { 1, 1, 0, 1 });
		const auto disjointReader = conservativeRangeGraph.AddPass("DisjointRangeReader", []() {});
		conservativeRangeGraph.Read(disjointReader, conservativeRangeResource,
			RenderGraph::ResourceState::ShaderRead, GraphicsPipelineStage::All, { 0, 1, 0, 1 });
		conservativeRangeGraph.AddDependency(disjointWriter, disjointReader);
		GE_INFO("Expected negative test: the next disjoint range cycle diagnostic is intentional.");
		if (conservativeRangeGraph.Compile())
			throw std::runtime_error("RenderGraph allowed independent disjoint subresource scheduling before backend support.");

		RenderGraph usageGraph;
		GraphicsPipelineStage observedStage = GraphicsPipelineStage::All;
		GraphicsResourceAccess observedAccess = GraphicsResourceAccess::ReadWrite;
		usageGraph.SetTransitionUsageCallback([&observedStage, &observedAccess](const FrameContext&,
			const Ref<GraphicsResource>&, const GraphicsResourceUsage&, const GraphicsResourceUsage& after,
			const RenderGraph::SubresourceRange&)
		{
			observedStage = after.Stage;
			observedAccess = after.Access;
		});
		auto usageStorage = usageGraph.ImportResource("UsageIntent", RenderGraph::ResourceState::Undefined);
		usageGraph.Write(usageGraph.AddPass("ComputeUsage", [] {}), usageStorage,
			RenderGraph::ResourceState::ShaderWrite, GraphicsPipelineStage::Compute);
		usageGraph.Execute();
		if (observedStage != GraphicsPipelineStage::Compute || observedAccess != GraphicsResourceAccess::Write)
			throw std::runtime_error("RenderGraph stage/access intent did not reach transition compilation.");

		RenderGraph subpassGraph;
		RenderGraph::AttachmentSpecification subpassAttachment{};
		subpassAttachment.ColorFormats = { FRAME_BUFFER_TEXTURE_FORMAT_RGBA8 };
		subpassAttachment.DepthStencilFormat = FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8;
		subpassAttachment.Subpasses.push_back({ { 0 }, {}, true });
		const auto subpassTarget = subpassGraph.CreateRenderTarget(
			"SubpassCapability", subpassAttachment, 16, 16);
		subpassGraph.BuildGraphicsPass("SubpassCapability", subpassTarget,
			[](const Ref<CommandBuffer>&) {});
		const bool supportsSubpasses = capabilities.Subpasses;
		bool subpassRejected = false;
		try
		{
			if (!subpassGraph.Compile())
				throw std::runtime_error("Render graph subpass capability compilation failed.");
			if (supportsSubpasses)
				subpassGraph.ExecuteGpu(GraphicsPresent::GetCommandBuffer());
		}
		catch (const std::invalid_argument&)
		{
			if (supportsSubpasses)
				throw;
			subpassRejected = true;
		}
		if (!supportsSubpasses && !subpassRejected)
			throw std::runtime_error("A backend without graph subpass support accepted subpass semantics.");
		subpassGraph.Reset();

		const float vertices[] =
		{
			 0.0f,  0.65f, 0.0f,
			 0.65f, -0.55f, 0.0f,
			-0.65f, -0.55f, 0.0f
		};

		auto shader = Shader::Create("Assets/Shaders/FrameGraphTriangle.shader");
		for (int request = 0; request < 64; ++request)
		{
			const auto cached = Shader::Create(request % 2
				? "Assets/Shaders/./FrameGraphTriangle.shader"
				: "Assets/Shaders/FrameGraphTriangle.shader");
			if (cached != shader)
				throw std::runtime_error("Shader factory cache identity check failed.");
		}
		auto& device = Graphics::GetRenderDevice();
		auto material = device.CreateMaterial(shader, "FrameGraphTriangleMaterial");
		m_Color = device.CreateStorageBuffer(16);
		const float initialColor[] = { 0.95f, 0.45f, 0.12f, 1.0f };
		m_Color->SetData(sizeof(initialColor), initialColor);
		material->SetResource("ColorData", m_Color);
		auto vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
		m_Pipeline = device.CreateGraphicsPipeline(material, vertexBuffer);
		m_ComputeFirstColor = device.CreateStorageBuffer(16);
		m_ComputeFirstColor->SetData(sizeof(initialColor), initialColor);
		auto computeFirstMaterial = device.CreateMaterial(shader, "ComputeFirstGraphics");
		computeFirstMaterial->SetResource("ColorData", m_ComputeFirstColor);
		m_ComputeFirstPipeline = device.CreateGraphicsPipeline(computeFirstMaterial, vertexBuffer);

		RenderGraph::AttachmentSpecification offscreenAttachment{};
		offscreenAttachment.ColorFormats = { FRAME_BUFFER_TEXTURE_FORMAT_RGBA8 };
		offscreenAttachment.DepthStencilFormat = FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8;
		const auto target = m_Graph.CreateRenderTarget("GraphColor", offscreenAttachment, 512, 512);
		m_Target = target;
		auto computeMaterial = device.CreateMaterial(Shader::Create("Assets/Shaders/FrameGraphColor.shader"), "GraphCompute");
		computeMaterial->SetResource("ColorData", m_Color);
		m_ComputePipeline = device.CreateComputePipeline(computeMaterial);
		auto computeFirstComputeMaterial = device.CreateMaterial(Shader::Create("Assets/Shaders/FrameGraphColor.shader"), "ComputeFirstCompute");
		computeFirstComputeMaterial->SetResource("ColorData", m_ComputeFirstColor);
		m_ComputeFirstComputePipeline = device.CreateComputePipeline(computeFirstComputeMaterial);
		const auto computeFirstInitial = m_Graph.GetVersion(m_Graph.ImportStorageBuffer(
			"ComputeFirstColor", m_ComputeFirstColor, RenderGraph::ResourceState::ShaderWrite));
		auto computeFirst = m_Graph.BuildComputePass("ComputeFirstColor", [this](const Ref<CommandBuffer>& command)
			{ command->Compute(m_ComputeFirstComputePipeline, 0, 1, 1, 1); });
		const auto computeFirstUpdated = computeFirst.Write(computeFirstInitial,
			RenderGraph::ResourceState::ShaderWrite, GraphicsPipelineStage::Compute);
		const auto initialVersion = m_Graph.GetVersion(m_Graph.ImportStorageBuffer("ColorData", m_Color, RenderGraph::ResourceState::ShaderWrite));
		auto first = m_Graph.BuildGraphicsPass("ReadPreviousColor", target, [this](const Ref<CommandBuffer>& command) { command->Render(m_Pipeline, 0); });
		first.Read(initialVersion, RenderGraph::ResourceState::ShaderWrite, GraphicsPipelineStage::Graphics);
		auto compute = m_Graph.BuildComputePass("UpdateColor", [this](const Ref<CommandBuffer>& command) { command->Compute(m_ComputePipeline, 0, 1, 1, 1); });
		auto updated = compute.Write(initialVersion, RenderGraph::ResourceState::ShaderWrite, GraphicsPipelineStage::Compute);
		auto final = m_Graph.BuildGraphicsPass("ReadUpdatedColor", target, [this](const Ref<CommandBuffer>& command)
			{ command->Render(m_Pipeline, 0); command->Render(m_ComputeFirstPipeline, 0); });
		final.Read(updated, RenderGraph::ResourceState::ShaderWrite, GraphicsPipelineStage::Graphics);
		final.Read(computeFirstUpdated, RenderGraph::ResourceState::ShaderWrite, GraphicsPipelineStage::Graphics);

		std::array<uint8_t, 4> sourcePixel{ 255, 0, 0, 255 };
		const std::array<uint8_t, 4> initialPixel{ 0, 0, 0, 255 };
		const std::array<uint8_t, 4> untouchedPixel{ 0, 255, 0, 255 };
		const Ref<Texture2D> rangeSource = Texture2D::Create(1, 1, sourcePixel.data(),
			static_cast<uint32_t>(sourcePixel.size()), RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM);
		const Ref<Texture2D> wrongSizeSource = Texture2D::Create(2, 1, RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM);
		const Ref<Texture2D> wrongFormatSource = Texture2D::Create(1, 1, RENDER_IMAGE_2D_FORMAT_RGBA8_SNORM);
		m_RangeArray = Texture2DArray::Create(1, 1, 2, RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM);
		m_RangeArray->SetData(initialPixel.data(), static_cast<uint32_t>(initialPixel.size()), 0);
		m_RangeArray->SetData(untouchedPixel.data(), static_cast<uint32_t>(untouchedPixel.size()), 1);
		const auto sourceResource = m_Graph.ImportTexture("SubresourceRangeSource", rangeSource,
			RenderGraph::ResourceState::ShaderRead);
		const auto rangeArrayResource = m_Graph.ImportTexture("SubresourceRangeArray", m_RangeArray,
			RenderGraph::ResourceState::ShaderRead);
		const auto copyLayerZero = m_Graph.BuildTransferCopyPass("CopyArrayLayerZero",
			[rangeSource, wrongSizeSource, wrongFormatSource, rangeArray = m_RangeArray](const Ref<CommandBuffer>& command)
			{
				auto& device = Graphics::GetRenderDevice();
				for (const auto& invalid : std::array<std::pair<Ref<Texture2D>, uint32_t>, 3>{
					std::pair{ rangeSource, 2u }, { wrongSizeSource, 0u }, { wrongFormatSource, 0u } })
				{
					bool rejected = false;
					try { device.CopyTexture2DToArrayLayer(command, invalid.first, rangeArray, invalid.second); }
					catch (const std::invalid_argument&) { rejected = true; }
					if (!rejected) throw std::runtime_error("Invalid texture-array copy was accepted.");
				}
				device.CopyTexture2DToArrayLayer(command, rangeSource, rangeArray, 0);
			}).GetHandle();
		m_Graph.Read(copyLayerZero, sourceResource, RenderGraph::ResourceState::CopySource,
			GraphicsPipelineStage::Transfer);
		m_Graph.Write(copyLayerZero, rangeArrayResource, RenderGraph::ResourceState::CopyDestination,
			GraphicsPipelineStage::Transfer, { 0, 1, 0, 1 });
		const auto layerReaders = m_Graph.BuildGraphicsPass("ArrayLayerReads", target,
			[](const Ref<CommandBuffer>&) {}).GetHandle();
		m_Graph.Read(layerReaders, rangeArrayResource, RenderGraph::ResourceState::ShaderRead,
			GraphicsPipelineStage::Graphics, { 0, 1, 1, 1 });
		m_Graph.Read(layerReaders, rangeArrayResource, RenderGraph::ResourceState::ShaderRead,
			GraphicsPipelineStage::Graphics, { 0, 1, 0, 1 });
		m_Graph.Read(layerReaders, sourceResource, RenderGraph::ResourceState::ShaderRead,
			GraphicsPipelineStage::Graphics);
	}

	void FrameGraphTriangleLayer::OnUpdate()
	{
		static const bool logStress = []()
		{
			const char* value = std::getenv("GENGINE_EXAMPLE_LOG_STRESS");
			return value && value[0] == '1' && value[1] == '\0';
		}();
		if (logStress && m_RenderCount == 30)
		{
			GE_INFO("Runtime log stress begins after {} frames.", m_RenderCount);
			auto writeRecords = [](int worker)
			{
				for (int record = 0; record < 40; ++record)
					GE_CORE_INFO("Runtime log stress worker {} record {}.", worker, record);
			};
			std::thread first(writeRecords, 1);
			std::thread second(writeRecords, 2);
			first.join();
			second.join();
			GE_INFO("Runtime log stress finished.");
		}
		static const bool syntheticResize = []()
		{
			const char* value = std::getenv("GENGINE_EXAMPLE_SYNTHETIC_RESIZE");
			return value != nullptr && value[0] == '1' && value[1] == '\0';
		}();
		if (!syntheticResize || Graphics::GetGraphicsAPI() != GRAPHICS_API_DIRECT3DX12 ||
			(m_RenderCount != 30 && m_RenderCount != 60))
			return;

		const uint32_t width = m_RenderCount == 30 ? 960 : 1280;
		const uint32_t height = m_RenderCount == 30 ? 540 : 720;
		WindowResizeEvent resize(width, height);
		Application::Get().OnEvent(resize);
		GE_INFO("Synthetic D3D12 resize requested: {}x{} after {} frames.", width, height, m_RenderCount);
	}

	void FrameGraphTriangleLayer::OnRender()
	{
		static const bool replaceResources = std::getenv("GENGINE_EXAMPLE_NO_REPLACEMENT") == nullptr;
		if (++m_RenderCount % 30 == 0 && replaceResources)
		{
			const float offset = (m_RenderCount / 30) % 2 ? 0.15f : -0.15f;
			const float vertices[] = {
				offset, 0.65f, 0.0f,
				0.65f + offset, -0.55f, 0.0f,
				-0.65f + offset, -0.55f, 0.0f
			};
			auto buffer = VertexBuffer::Create(vertices, sizeof(vertices));
			auto shader = Shader::Create("Assets/Shaders/FrameGraphTriangle.shader");
			auto material = Material::Create(shader, "ReplacementMaterial");
			material->SetResource("ColorData", m_Color);
			m_Pipeline = GraphicsPipeline::Create(material, buffer);
			// Preserve the render-pass replacement regression from the other branch.
			auto frameBuffer = m_Graph.GetFrameBuffer(m_Target);
			frameBuffer->SetRenderPassOperation(frameBuffer->GetRenderPass()->GetSpecification().Operation);
		}
		m_Graph.ExecuteGpu(GraphicsPresent::GetCommandBuffer());
		if (m_RenderCount == 119)
		{
			float result[4]{};
			m_Color->ReadData(sizeof(result), result);
			if (std::abs(result[0] - 0.15f) > 0.001f || std::abs(result[1] - 0.45f) > 0.001f ||
				std::abs(result[2] - 0.12f) > 0.001f || std::abs(result[3] - 1.0f) > 0.001f)
				throw std::runtime_error("GPU graph compute readback mismatch.");
			GE_INFO("GPU graph compute readback passed after 119 frames.");
			float computeFirstResult[4]{};
			m_ComputeFirstColor->ReadData(sizeof(computeFirstResult), computeFirstResult);
			if (std::abs(computeFirstResult[0] - 0.15f) > 0.001f ||
				std::abs(computeFirstResult[1] - 0.45f) > 0.001f ||
				std::abs(computeFirstResult[2] - 0.12f) > 0.001f ||
				std::abs(computeFirstResult[3] - 1.0f) > 0.001f)
				throw std::runtime_error("Compute-first GPU graph readback mismatch.");
			GE_INFO("Compute-first GPU graph readback passed after 119 frames.");
			std::array<uint8_t, 4> copiedPixel{};
			std::array<uint8_t, 4> preservedPixel{};
			m_RangeArray->ReadLayerData(0, static_cast<uint32_t>(copiedPixel.size()), copiedPixel.data());
			m_RangeArray->ReadLayerData(1, static_cast<uint32_t>(preservedPixel.size()), preservedPixel.data());
			if (copiedPixel != std::array<uint8_t, 4>{ 255, 0, 0, 255 } ||
				preservedPixel != std::array<uint8_t, 4>{ 0, 255, 0, 255 })
				throw std::runtime_error("GPU graph texture-array layer readback mismatch.");
			GE_INFO("GPU graph texture-array layer readback passed after 119 frames.");
		}
	}

	void FrameGraphTriangleLayer::OnPresent()
	{
		// RenderSystem imports the presentation target, records the portable
		// Present -> RenderTarget -> Present state usage, then dispatches this draw.
		GraphicsPresent::Render(m_Pipeline, 0);
		static const uint64_t frameLimit = []()
		{
			const auto* value = std::getenv("GENGINE_EXAMPLE_FRAME_LIMIT");
			return value ? std::strtoull(value, nullptr, 10) : 0ull;
		}();
		static const double secondsLimit = []()
		{
			const auto* value = std::getenv("GENGINE_EXAMPLE_SECONDS");
			return value ? std::strtod(value, nullptr) : 0.0;
		}();
		static const auto firstFrameTime = std::chrono::steady_clock::now();
		const auto elapsedSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - firstFrameTime).count();
		if ((frameLimit && m_RenderCount >= frameLimit) ||
			(secondsLimit > 0.0 && elapsedSeconds >= secondsLimit))
		{
            if (std::getenv("GENGINE_EXAMPLE_RESOURCE_CHECK") && HasResourcePack())
            {
                if (std::filesystem::exists("Assets")) throw std::runtime_error("Asset extraction appeared during rendering.");
                GE_INFO("Resource archive check passed at shutdown: no extracted Assets; writable cache and logs remain separate.");
            }
			GE_INFO("FrameGraphTriangle completed {} frames; requesting normal shutdown.", m_RenderCount);
			Application::Get().Close();
		}
	}
}
