#include "FrameGraphTriangleLayer.h"
#include <GEngine/Renderer/RenderGraph.h>
#include <stdexcept>
#include <cstdlib>
#include <chrono>

namespace GEngine
{
	FrameGraphTriangleLayer::FrameGraphTriangleLayer()
		: Layer("FrameGraphTriangle")
	{
	}

	void FrameGraphTriangleLayer::OnAttach()
	{
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

		RenderPassSpecification offscreenSpecification{};
		offscreenSpecification.RenderTargets = { FRAME_BUFFER_TEXTURE_FORMAT_RGBA8 };
		offscreenSpecification.DepthStencil = FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8;
		const auto target = m_Graph.CreateRenderTarget("GraphColor", offscreenSpecification, 512, 512);
		m_Target = target;
		auto computeMaterial = device.CreateMaterial(Shader::Create("Assets/Shaders/FrameGraphColor.shader"), "GraphCompute");
		computeMaterial->SetResource("ColorData", m_Color);
		m_ComputePipeline = device.CreateComputePipeline(computeMaterial);
		const auto initialVersion = m_Graph.GetVersion(m_Graph.ImportStorageBuffer("ColorData", m_Color, RenderGraph::ResourceState::ShaderWrite));
		auto first = m_Graph.BuildGraphicsPass("ReadPreviousColor", target, [this](const Ref<CommandBuffer>& command) { command->Render(m_Pipeline, 0); });
		first.Read(initialVersion, RenderGraph::ResourceState::ShaderWrite);
		auto compute = m_Graph.BuildComputePass("UpdateColor", [this](const Ref<CommandBuffer>& command) { command->Compute(m_ComputePipeline, 0, 1, 1, 1); });
		auto updated = compute.Write(initialVersion, RenderGraph::ResourceState::ShaderWrite);
		auto final = m_Graph.BuildGraphicsPass("ReadUpdatedColor", target, [this](const Ref<CommandBuffer>& command) { command->Render(m_Pipeline, 0); });
		final.Read(updated, RenderGraph::ResourceState::ShaderWrite);
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
			GE_INFO("FrameGraphTriangle completed {} frames; requesting normal shutdown.", m_RenderCount);
			Application::Get().Close();
		}
	}
}
