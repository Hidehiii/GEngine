#include "FrameGraphTriangleLayer.h"
#include <GEngine/Renderer/RenderGraph.h>
#include <stdexcept>

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
		const float vertices[] =
		{
			 0.0f,  0.65f, 0.0f,
			 0.65f, -0.55f, 0.0f,
			-0.65f, -0.55f, 0.0f
		};

		auto shader = Shader::Create("Assets/Shaders/FrameGraphTriangle.shader");
		auto material = Material::Create(shader, "FrameGraphTriangleMaterial");
		auto vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
		m_Pipeline = GraphicsPipeline::Create(material, vertexBuffer);

		RenderPassSpecification offscreenSpecification{};
		offscreenSpecification.RenderTargets = { FRAME_BUFFER_TEXTURE_FORMAT_RGBA8 };
		offscreenSpecification.DepthStencil = FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8;
		m_OffscreenRenderPass = RenderPass::Create(offscreenSpecification);
		m_OffscreenFrameBuffer = FrameBuffer::Create(m_OffscreenRenderPass, 512, 512);
	}

	void FrameGraphTriangleLayer::OnRender()
	{
		if (++m_RenderCount % 30 == 0)
		{
			const float offset = (m_RenderCount / 30) % 2 ? 0.15f : -0.15f;
			const float vertices[] = {
				offset, 0.65f, 0.0f,
				0.65f + offset, -0.55f, 0.0f,
				-0.65f + offset, -0.55f, 0.0f
			};
			auto buffer = VertexBuffer::Create(vertices, sizeof(vertices));
			m_Pipeline = GraphicsPipeline::Create(m_Pipeline->GetMaterial(), buffer);
		}
		auto commandBuffer = Graphics::GetGraphicsCommandBuffer();
		auto& graphicsQueue = Graphics::GetRenderDevice().GetQueue(COMMAND_BUFFER_TYPE_GRAPHICS);

		Graphics::SetCommandsBarrier(commandBuffer, GraphicsPresent::GetCommandBuffer());
		commandBuffer->Begin(m_OffscreenFrameBuffer);
		commandBuffer->Render(m_Pipeline, 0);
		commandBuffer->End();
		graphicsQueue.Submit(commandBuffer);
	}

	void FrameGraphTriangleLayer::OnPresent()
	{
		// RenderSystem imports the presentation target, records the portable
		// Present -> RenderTarget -> Present state usage, then dispatches this draw.
		GraphicsPresent::Render(m_Pipeline, 0);
	}
}
