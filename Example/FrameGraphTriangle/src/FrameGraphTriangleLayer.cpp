#include "FrameGraphTriangleLayer.h"

namespace GEngine
{
	FrameGraphTriangleLayer::FrameGraphTriangleLayer()
		: Layer("FrameGraphTriangle")
	{
	}

	void FrameGraphTriangleLayer::OnAttach()
	{
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
