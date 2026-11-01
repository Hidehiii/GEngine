#include "MsaaChecks.h"
#include <GEngine.h>
#include <GEngine/Graphics/FrameBuffer.h>
#include <GEngine/Graphics/GraphicsPresent.h>
#include <GEngine/Graphics/Texture.h>
#include <GEngine/Renderer/RenderGraph.h>
#include <array>
#include <stdexcept>
#include <vector>

namespace GEngine
{
	static std::vector<uint8_t> CheckDepthPixels(const Ref<Texture2D>& depth, const Ref<Texture2D>& storedColor,
		uint32_t width, uint32_t height, float expected, bool uniform = false)
	{
		auto& device = Graphics::GetRenderDevice();
		RenderGraph graph;
		RenderGraph::AttachmentSpecification spec{};
		spec.ColorFormats = { FRAME_BUFFER_TEXTURE_FORMAT_RGBA8 };
		const auto target = graph.CreateRenderTarget("DepthProbe", spec, width, height);
		const auto source = graph.ImportTexture("ResolvedDepth", depth, RenderGraph::ResourceState::ShaderRead);
		const auto stored = graph.ImportTexture("StoredColor", storedColor, RenderGraph::ResourceState::ShaderRead);
		auto material = device.CreateMaterial(Shader::Create("Assets/Shaders/DepthResolveCheck.gasset"), "DepthProbe");
		material->SetResource<Texture2D>("DepthTexture", depth);
		material->SetResource<Texture2D>("StoredColor", storedColor);
		SamplerSpecification nearest{};
		nearest.MinFilter = nearest.MagFilter = nearest.MipmapFilter = SAMPLER_FILTER_NEAREST;
		const auto sampler = device.CreateSampler(nearest);
		material->SetResource<Sampler>("DepthSampler", sampler);
		material->SetResource<Sampler>("ColorSampler", sampler);
		material->SetConstant<Vector2>("InvSize", Vector2{ 1.0f / width, 1.0f / height });
		const float vertices[] = { -1, -1, 3, -1, -1, 3 };
		auto pipeline = device.CreateGraphicsPipeline(material, device.CreateVertexBuffer(vertices, sizeof(vertices), 0, VERTEX_TOPOLOGY_TRIANGLE));
		const auto draw = graph.BuildGraphicsPass("SampleResolvedDepth", target, [&](const Ref<CommandBuffer>& cmd) { cmd->Render(pipeline, 0); }).GetHandle();
		graph.Read(draw, source, RenderGraph::ResourceState::ShaderRead, GraphicsPipelineStage::FragmentShader);
		graph.Read(draw, stored, RenderGraph::ResourceState::ShaderRead, GraphicsPipelineStage::FragmentShader);
		auto array = device.CreateTexture2DArray(width, height, 1, RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM);
		const auto destination = graph.ImportTexture("DepthReadback", array, RenderGraph::ResourceState::ShaderRead);
		const auto color = graph.GetColorAttachment(target);
		const auto copy = graph.BuildTransferCopyPass("CopyDepthProbe", [&](const Ref<CommandBuffer>& cmd)
		{ device.CopyTexture2DToArrayLayer(cmd, graph.GetTexture2D(color), array, 0); }).GetHandle();
		graph.Read(copy, color, RenderGraph::ResourceState::CopySource, GraphicsPipelineStage::Transfer);
		graph.Write(copy, destination, RenderGraph::ResourceState::CopyDestination, GraphicsPipelineStage::Transfer);
		const auto readable = graph.BuildGraphicsCopyPass("DepthProbeReadable", [](const Ref<CommandBuffer>&) {}).GetHandle();
		graph.Read(readable, color, RenderGraph::ResourceState::ShaderRead, GraphicsPipelineStage::FragmentShader);
		graph.Read(readable, destination, RenderGraph::ResourceState::ShaderRead, GraphicsPipelineStage::FragmentShader);
		graph.ExecuteGpu(GraphicsPresent::GetCommandBuffer());
		std::vector<uint8_t> pixels(width * height * 4);
		array->ReadLayerData(0, static_cast<uint32_t>(pixels.size()), pixels.data());
		const auto center = ((height / 2) * width + width / 2) * 4;
		const int expectedByte = static_cast<int>(expected * 255.0f + 0.5f);
		const int clearByte = uniform ? expectedByte : (Graphics::IsReverseDepth() ? 0 : 255);
		if (std::abs(static_cast<int>(pixels[center]) - expectedByte) > 2 || std::abs(static_cast<int>(pixels[0]) - clearByte) > 2)
			throw std::runtime_error("Resolved depth sampling mismatch: center=" + std::to_string(pixels[center]) + ", expected=" + std::to_string(expectedByte));
		GE_INFO("MSAA numeric depth passed: center={}, clear={}, reverse={}.", pixels[center], pixels[0], Graphics::IsReverseDepth());
		if (pixels[center + 1] < 250 || pixels[center + 2] > 5)
			throw std::runtime_error("Stored MSAA color was lost during LOAD.");
		return pixels;
	}

	class MsaaFrameChecksImpl final : public MsaaFrameChecks
	{
	public:
		void Run(uint64_t frame) override
		{
			auto& device = Graphics::GetRenderDevice();
			if (frame == 0 || frame == 60)
			{
				m_Graph.Reset();
				m_Width = frame ? 47 : 31;
				m_Height = frame ? 33 : 29;
				RenderGraph::AttachmentSpecification spec{};
				spec.ColorFormats = { FRAME_BUFFER_TEXTURE_FORMAT_RGBA8, FRAME_BUFFER_TEXTURE_FORMAT_RGBA8 };
				spec.DepthStencilFormat = FRAME_BUFFER_TEXTURE_FORMAT_DEPTH;
				spec.Samples = 4;
				if (!device.QueryRenderTargetFormat(spec.DepthStencilFormat).SupportsSamples(4))
					throw std::runtime_error("Cross-frame MSAA depth check requires queried 4x support.");
				m_Target = m_Graph.CreateRenderTarget("CrossFrameMsaa", spec, m_Width, m_Height);
				auto material = device.CreateMaterial(Shader::Create("Assets/Shaders/MsaaCheck.gasset"), "CrossFrameMsaa");
				material->SetDepthTestOp(Graphics::IsReverseDepth() ? COMPARE_OP_GREATER : COMPARE_OP_LESS, 0);
				material->SetEnableDepthWrite(true, 0);
				const bool gl = Graphics::GetGraphicsAPI() == GRAPHICS_API_OPENGL;
				m_Depth = Graphics::IsReverseDepth() ? 0.8f : 0.2f;
				const float nearZ = gl ? -0.6f : m_Depth, farZ = gl ? 0.6f : 1.0f - m_Depth;
				float vertices[] = { -1,-1,nearZ, 1,0,0,1, 0,1,0,1,
					3,-1,nearZ, 1,0,0,1, 0,1,0,1, -1,3,nearZ, 1,0,0,1, 0,1,0,1 };
				m_Near = device.CreateGraphicsPipeline(material, device.CreateVertexBuffer(vertices, sizeof(vertices), 0, VERTEX_TOPOLOGY_TRIANGLE));
				for (size_t i = 0; i < 33; i += 11) { vertices[i+2] = farZ; vertices[i+3] = 0; vertices[i+5] = 1; vertices[i+8] = 0; vertices[i+9] = 1; }
				m_Far = device.CreateGraphicsPipeline(material, device.CreateVertexBuffer(vertices, sizeof(vertices), 0, VERTEX_TOPOLOGY_TRIANGLE));
				m_Graph.BuildGraphicsPass("CrossFrameGeometry", m_Target, [this](const Ref<CommandBuffer>& cmd)
				{ if (m_First) cmd->Render(m_Near, 0); else cmd->Render(m_Far, 0); });
				m_First = true;
			}
			m_Graph.ExecuteGpu(GraphicsPresent::GetCommandBuffer());
			if (m_First)
			{
				auto load = m_Graph.GetFrameBuffer(m_Target)->GetRenderPass()->GetSpecification().Operation;
				load.ColorBegin = load.DepthStencilBegin = RENDER_PASS_BEGINE_OP_LOAD;
				m_Graph.GetFrameBuffer(m_Target)->SetRenderPassOperation(load);
				m_First = false;
			}
			if (frame == 1 || frame == 59 || frame == 61 || frame == 119)
			{
				auto framebuffer = m_Graph.GetFrameBuffer(m_Target);
				CheckDepthPixels(framebuffer->GetDepthStencil(), framebuffer->GetRenderTarget(0), m_Width, m_Height, m_Depth, true);
				GE_INFO("MSAA cross-frame LOAD passed at frame {} ({}x{}).", frame, m_Width, m_Height);
			}
		}
	private:
		RenderGraph m_Graph;
		RenderGraph::TargetHandle m_Target = RenderGraph::InvalidTarget;
		Ref<GraphicsPipeline> m_Near, m_Far;
		bool m_First = true;
		uint32_t m_Width = 0, m_Height = 0;
		float m_Depth = 0;
	};
	std::shared_ptr<MsaaFrameChecks> CreateMsaaFrameChecks() { return std::make_shared<MsaaFrameChecksImpl>(); }

	void RunMsaaChecks()
	{
		auto& device = Graphics::GetRenderDevice();
		for (uint32_t value = 0; value <= FRAME_BUFFER_TEXTURE_FORMAT_RGBA16F; ++value)
		{
			const auto support = device.QueryRenderTargetFormat(static_cast<FrameBufferTextureFormat>(value));
			GE_INFO("MSAA format {}: color={}, depth={}, sampled={}, colorResolve={}, depthResolve={}, samples={}, max={}x{}, load={}.",
				value, support.ColorAttachment, support.DepthStencilAttachment, support.Sampled,
				support.ColorResolve, support.DepthResolve, support.SampleCounts, support.MaxWidth, support.MaxHeight, support.MultisampleLoad);
			if (support.ColorAttachment && support.Sampled && support.SupportsSamples(1))
			{
				RenderPassSpecification spec{};
				spec.RenderTargets = { static_cast<FrameBufferTextureFormat>(value) };
				auto framebuffer = device.CreateFrameBuffer(device.CreateRenderPass(spec), 8, 8);
				const auto texture = framebuffer->GetRenderTarget(0);
				const auto expected = value == FRAME_BUFFER_TEXTURE_FORMAT_RGBA8 ?
					RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM : RENDER_IMAGE_2D_FORMAT_NONE;
				if (texture->GetWidth() != 8 || texture->GetHeight() != 8 || texture->GetFormat() != expected)
					throw std::runtime_error("Per-format framebuffer metadata is inconsistent.");
			}
		}
		if (device.QueryRenderTargetFormat(static_cast<FrameBufferTextureFormat>(999)).SampleCounts ||
			device.QueryRenderTargetFormat(FRAME_BUFFER_TEXTURE_FORMAT_NONE).SampleCounts)
			throw std::runtime_error("Invalid attachment formats reported sample support.");
		const auto colorSupport = device.QueryRenderTargetFormat(FRAME_BUFFER_TEXTURE_FORMAT_RGBA8);
		for (auto depth : { FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8, FRAME_BUFFER_TEXTURE_FORMAT_DEPTH })
		{
			const auto support = device.QueryRenderTargetFormat(depth);
			if (!support.SupportsSamples(4) || !colorSupport.SupportsSamples(4)) continue;
			for (auto mode : { DepthResolveMode::BackendDefault, DepthResolveMode::SampleZero, DepthResolveMode::Minimum,
				DepthResolveMode::Maximum, static_cast<DepthResolveMode>(3) })
			{
				if (support.SupportsDepthResolve(mode)) continue;
				RenderGraph invalid;
				const auto sentinel = invalid.CreateTransientStorageBuffer("DepthModePreflight", { 16 });
				RenderGraph::AttachmentSpecification spec{};
				spec.ColorFormats = { FRAME_BUFFER_TEXTURE_FORMAT_RGBA8 };
				spec.DepthStencilFormat = depth;
				spec.Samples = 4;
				spec.DepthResolve = mode;
				invalid.CreateRenderTarget("UnsupportedDepthMode", spec, 8, 8);
				bool rejected = false;
				try { invalid.Compile(); } catch (const std::invalid_argument&) { rejected = true; }
				if (!rejected || invalid.GetStorageBuffer(sentinel))
					throw std::runtime_error("Unsupported depth resolve mode was not rejected before allocation.");
				GE_INFO("MSAA unsupported depth mode rejected: format={}, mode={}.", static_cast<int>(depth), static_cast<uint32_t>(mode));
			}
		}
		if (colorSupport.SupportsSamples(4) && !colorSupport.MultisampleLoad)
		{
			RenderGraph invalid;
			const auto sentinel = invalid.CreateTransientStorageBuffer("MsaaLoadPreflight", { 16 });
			RenderGraph::AttachmentSpecification spec{};
			spec.ColorFormats = { FRAME_BUFFER_TEXTURE_FORMAT_RGBA8 };
			spec.Samples = 4;
			spec.Operation.ColorBegin = RENDER_PASS_BEGINE_OP_LOAD;
			invalid.CreateRenderTarget("UnsupportedMsaaLoad", spec, 8, 8);
			bool rejected = false;
			try { invalid.Compile(); }
			catch (const std::invalid_argument&) { rejected = true; }
			if (!rejected || invalid.GetStorageBuffer(sentinel))
				throw std::runtime_error("Unsupported multisample LOAD was not rejected before allocation.");
			GE_INFO("MSAA preserved LOAD rejected before allocation as reported.");
		}
		for (uint32_t samples : { 1u, 2u, 4u, 8u, 16u, 32u, 64u })
		{
			if (colorSupport.SupportsSamples(samples) && (samples == 1 || colorSupport.ColorResolve)) continue;
			RenderGraph invalid;
			const auto sentinel = invalid.CreateTransientStorageBuffer("MsaaPreflight", { 16 });
			RenderGraph::AttachmentSpecification spec{};
			spec.ColorFormats = { FRAME_BUFFER_TEXTURE_FORMAT_RGBA8 };
			spec.Samples = samples;
			invalid.CreateRenderTarget("UnsupportedSamples", spec, 8, 8);
			bool rejected = false;
			try { invalid.Compile(); }
			catch (const std::invalid_argument&) { rejected = true; }
			if (!rejected || invalid.GetStorageBuffer(sentinel))
				throw std::runtime_error("Unsupported MSAA allocated graph resources.");
			RenderPassSpecification direct{};
			direct.RenderTargets = spec.ColorFormats;
			direct.Samples = samples;
			rejected = false;
			try { device.CreateRenderPass(direct); }
			catch (const std::invalid_argument&) { rejected = true; }
			if (!rejected) throw std::runtime_error("Direct factory accepted unsupported MSAA.");
			GE_INFO("MSAA unsupported {}x rejected before allocation.", samples);
		}

		uint32_t cases = 0, multisampledCases = 0;
		const bool originalReverse = Graphics::IsReverseDepth();
		struct RestoreDepth { bool Value; ~RestoreDepth() { Graphics::SetReverseDepth(Value); } } restore{ originalReverse };
		for (bool reverse : { false, true })
		{
		Graphics::SetReverseDepth(reverse);
		for (const auto depth : { FRAME_BUFFER_TEXTURE_FORMAT_NONE, FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8,
			FRAME_BUFFER_TEXTURE_FORMAT_DEPTH })
		{
			const auto depthSupport = device.QueryRenderTargetFormat(depth);
			for (auto mode : { DepthResolveMode::BackendDefault, DepthResolveMode::SampleZero, DepthResolveMode::Minimum, DepthResolveMode::Maximum })
			{
			if (mode != DepthResolveMode::BackendDefault && !depthSupport.SupportsDepthResolve(mode)) continue;
			for (const uint32_t samples : { 1u, 2u, 4u, 8u })
			{
				if (mode != DepthResolveMode::BackendDefault && samples != 4) continue;
				if (!colorSupport.SupportsSamples(samples) || (samples > 1 && !colorSupport.ColorResolve) ||
					(depth != FRAME_BUFFER_TEXTURE_FORMAT_NONE && !depthSupport.SupportsSamples(samples)))
				{
					GE_INFO("MSAA case skipped: depth={}, samples={} (queried unsupported).", static_cast<int>(depth), samples);
					continue;
				}
				for (uint32_t generation = 0; generation < 2; ++generation)
				{
					const uint32_t width = generation ? 47 : 31, height = generation ? 33 : 29;
					RenderGraph graph;
					RenderGraph::AttachmentSpecification spec{};
					spec.ColorFormats = { FRAME_BUFFER_TEXTURE_FORMAT_RGBA8, FRAME_BUFFER_TEXTURE_FORMAT_RGBA8 };
					spec.DepthStencilFormat = depth;
					spec.Samples = samples;
					spec.DepthResolve = mode;
					const auto target = graph.CreateRenderTarget("MsaaMrt", spec, width, height);
					auto material = device.CreateMaterial(Shader::Create("Assets/Shaders/MsaaCheck.gasset"), "MsaaCheck");
					const bool hasDepth = depth != FRAME_BUFFER_TEXTURE_FORMAT_NONE;
					material->SetDepthTestOp(hasDepth ? (Graphics::IsReverseDepth() ? COMPARE_OP_GREATER : COMPARE_OP_LESS) : COMPARE_OP_ALWAYS, 0);
					material->SetEnableDepthWrite(hasDepth, 0);
					struct Vertex { float x, y, z; float r, g, b, a; float r1, g1, b1, a1; };
					const float nearZ = Graphics::IsReverseDepth() ? 0.8f : 0.2f;
					const bool glDepthRange = Graphics::GetGraphicsAPI() == GRAPHICS_API_OPENGL;
					const float clipNear = glDepthRange ? -0.6f : nearZ;
					Vertex vertices[] = {
						{-0.72f, -0.65f, clipNear, 1, 0, 0, 1, 0, 1, 0, 1},
						{ 0.68f, -0.44f, clipNear, 1, 0, 0, 1, 0, 1, 0, 1},
						{-0.12f,  0.78f, clipNear, 1, 0, 0, 1, 0, 1, 0, 1} };
					auto nearPipeline = device.CreateGraphicsPipeline(material, device.CreateVertexBuffer(vertices, sizeof(vertices), 0, VERTEX_TOPOLOGY_TRIANGLE));
					for (auto& vertex : vertices)
					{
						vertex.z = glDepthRange ? 0.6f : 1.0f - nearZ;
						vertex.r = vertex.g1 = 0;
						vertex.b = vertex.b1 = 1;
					}
					auto farPipeline = device.CreateGraphicsPipeline(material, device.CreateVertexBuffer(vertices, sizeof(vertices), 0, VERTEX_TOPOLOGY_TRIANGLE));
					bool drawNear = true;
					graph.BuildGraphicsPass("MsaaGeometry", target, [&](const Ref<CommandBuffer>& command)
					{
						if (drawNear) command->Render(nearPipeline, 0);
						if (hasDepth) command->Render(farPipeline, 0);
					});
					auto array = device.CreateTexture2DArray(width, height, 2, RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM);
					const auto destination = graph.ImportTexture("MsaaReadback", array, RenderGraph::ResourceState::ShaderRead);
					const auto copy = graph.BuildTransferCopyPass("MsaaResolveCopy", [&](const Ref<CommandBuffer>& command)
					{
						for (uint32_t index = 0; index < 2; ++index)
							device.CopyTexture2DToArrayLayer(command, graph.GetTexture2D(graph.GetColorAttachment(target, index)), array, index);
					}).GetHandle();
					for (uint32_t index = 0; index < 2; ++index)
						graph.Read(copy, graph.GetColorAttachment(target, index), RenderGraph::ResourceState::CopySource, GraphicsPipelineStage::Transfer);
					graph.Write(copy, destination, RenderGraph::ResourceState::CopyDestination, GraphicsPipelineStage::Transfer);
					// Close the copy-queue chain before reusing the graph in this diagnostic frame.
					const auto readable = graph.BuildGraphicsCopyPass("MsaaReadable", [](const Ref<CommandBuffer>&) {}).GetHandle();
					for (uint32_t index = 0; index < 2; ++index)
						graph.Read(readable, graph.GetColorAttachment(target, index), RenderGraph::ResourceState::ShaderRead,
							GraphicsPipelineStage::FragmentShader);
					graph.Read(readable, destination, RenderGraph::ResourceState::ShaderRead, GraphicsPipelineStage::FragmentShader);
					graph.ExecuteGpu(GraphicsPresent::GetCommandBuffer());
					if (samples > 1 && colorSupport.MultisampleLoad && (!hasDepth || depthSupport.MultisampleLoad))
					{
						auto load = spec.Operation;
						load.ColorBegin = RENDER_PASS_BEGINE_OP_LOAD;
						load.DepthStencilBegin = RENDER_PASS_BEGINE_OP_LOAD;
						graph.GetFrameBuffer(target)->SetRenderPassOperation(load);
						drawNear = false;
					}
					for (uint32_t repeat = 0; repeat < 2; ++repeat) graph.ExecuteGpu(GraphicsPresent::GetCommandBuffer());
					std::vector<uint8_t> first(width * height * 4), second(first.size());
					array->ReadLayerData(0, static_cast<uint32_t>(first.size()), first.data());
					array->ReadLayerData(1, static_cast<uint32_t>(second.size()), second.data());
					const auto center = ((height / 2) * width + width / 2) * 4;
					if (first[center] < 250 || first[center + 1] > 5 || first[center + 2] > 5 ||
						second[center + 1] < 250 || second[center] > 5 || second[center + 2] > 5)
						throw std::runtime_error("MSAA MRT resolve or depth-occlusion pixel mismatch.");
					uint32_t fractional = 0;
					for (size_t pixel = 0; pixel < first.size(); pixel += 4)
						if (first[pixel] > first[0] + 4 && first[pixel] < 251 && first[pixel + 2] < 250) ++fractional;
					if (samples > 1 && !fractional) throw std::runtime_error("MSAA resolve produced no fractional edge coverage.");
					auto framebuffer = graph.GetFrameBuffer(target);
					if (hasDepth && (samples == 1 || depthSupport.DepthResolve))
					{
						const auto depths = CheckDepthPixels(framebuffer->GetDepthStencil(), framebuffer->GetRenderTarget(0), width, height, nearZ);
						if (samples > 1 && (mode == DepthResolveMode::Minimum || mode == DepthResolveMode::Maximum ||
							(mode == DepthResolveMode::BackendDefault && Graphics::GetGraphicsAPI() == GRAPHICS_API_DIRECT3DX12)))
						{
							const bool maximum = mode == DepthResolveMode::Maximum || (mode == DepthResolveMode::BackendDefault && reverse);
							const float clear = reverse ? 0.0f : 1.0f;
							const int edge = static_cast<int>((maximum ? std::max(nearZ, clear) : std::min(nearZ, clear)) * 255.0f + 0.5f);
							uint32_t checked = 0;
							for (size_t pixel = 0; pixel < first.size(); pixel += 4)
								if (first[pixel] > first[0] + 4 && first[pixel] < 251 && first[pixel + 2] < 250)
								{
									if (std::abs(static_cast<int>(depths[pixel]) - edge) > 2)
										throw std::runtime_error("MSAA depth edge reduction does not match the requested min/max mode.");
									++checked;
								}
							if (!checked) throw std::runtime_error("MSAA depth reduction did not exercise mixed coverage.");
							GE_INFO("MSAA depth edge reduction passed: mode={}, reverse={}, pixels={}.", static_cast<uint32_t>(mode), reverse, checked);
						}
					}
					if (framebuffer->GetRenderTargetCount() != 2)
						throw std::runtime_error("MSAA exposed backing images as extra color attachments.");
					auto retained = framebuffer->GetRenderTarget(0);
					if (retained.owner_before(framebuffer) || framebuffer.owner_before(retained))
						throw std::runtime_error("Attachment view does not retain its framebuffer owner.");
					if (generation == 0)
					{
						auto resized = device.ResizeFrameBuffer(framebuffer, width + 4, height + 2);
						if (resized->GetSamples() != samples || resized->GetRenderTarget(1)->GetWidth() != width + 4)
							throw std::runtime_error("MSAA device resize changed the sample count or dimensions.");
					}
					if (hasDepth && samples > 1 && !depthSupport.DepthResolve)
					{
						bool rejected = false;
						try { framebuffer->GetDepthStencil(); }
						catch (const std::invalid_argument&) { rejected = true; }
						if (!rejected) throw std::runtime_error("Unsupported depth resolve was silently exposed.");
					}
					framebuffer.reset();
					graph.Reset();
					if (retained->GetWidth() != width || retained->GetHeight() != height)
						throw std::runtime_error("Attachment metadata did not survive graph reset.");
					GE_INFO("MSAA pixels passed: {}x, depth={}, {}x{}, MRT=2, edgePixels={}.", samples, static_cast<int>(depth), width, height, fractional);
					++cases;
					if (samples > 1) ++multisampledCases;
				}
			}
			}
		}
		}
		if (!multisampledCases) throw std::runtime_error("No supported multisample workload was exercised.");
		GE_INFO("MSAA matrix passed: {} cases, {} multisample cases.", cases, multisampledCases);
	}
}
