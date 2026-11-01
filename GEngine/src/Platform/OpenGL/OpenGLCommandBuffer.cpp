#include "GEpch.h"
#include "Platform/OpenGL/OpenGLCommandBuffer.h"
#include "Platform/OpenGL/OpenGLGraphicsPipeline.h"
#include "Platform/OpenGL/OpenGLComputePipeline.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Math/Math.h"
#include <glad/glad.h>

namespace GEngine
{
	void OpenGLCommandBuffer::BeginRenderPass(Ref<FrameBuffer>& buffer)
	{
		BeginRenderPass(buffer, buffer ? buffer->GetRenderPass() : nullptr);
	}
	void OpenGLCommandBuffer::BeginRenderPass(Ref<FrameBuffer>& buffer, const Ref<RenderPass>& renderPass)
	{
		if (!buffer || m_Type != COMMAND_BUFFER_TYPE_GRAPHICS || m_FrameBuffer)
			throw std::invalid_argument("Invalid render-pass begin.");
		buffer->ValidateRenderPass(renderPass);
		m_FrameBuffer = std::static_pointer_cast<OpenGLFrameBuffer>(buffer);
		m_FrameBuffer->Begin(this, renderPass);
		m_Viewport = { 0, 0, buffer->GetSpecification().Width, buffer->GetSpecification().Height };
	}
	void OpenGLCommandBuffer::EndRenderPass()
	{
		if (m_FrameBuffer) m_FrameBuffer->End(this);
		m_FrameBuffer.reset();
	}
	void OpenGLCommandBuffer::SetViewport(const GraphicsViewport& viewport)
	{
		CheckViewport(viewport, m_FrameBuffer);
		m_Viewport = viewport;
		const auto bottom = m_FrameBuffer->GetSpecification().Height - viewport.Y - viewport.Height;
		glViewport(viewport.X, bottom, viewport.Width, viewport.Height);
		glEnable(GL_SCISSOR_TEST);
		glScissor(viewport.X, bottom, viewport.Width, viewport.Height);
	}
	OpenGLCommandBuffer::OpenGLCommandBuffer(CommandBufferType type)
	{
		m_Type = type;
	}
	Ref<OpenGLCommandBuffer> OpenGLCommandBuffer::Create(CommandBufferType type)
	{
		return CreateRef<OpenGLCommandBuffer>(type);
	}
	void OpenGLCommandBuffer::BeginPresentRender(Ref<FrameBuffer>& buffer)
	{
		if (m_Type == COMMAND_BUFFER_TYPE_GRAPHICS)
		{
			GE_CORE_ASSERT(buffer != nullptr, "graphics cmd must have frame buffer");
			m_FrameBuffer = std::static_pointer_cast<OpenGLFrameBuffer>(buffer);
			m_FrameBuffer->BeginPresentRender(this);
			m_Viewport = { 0, 0, buffer->GetSpecification().Width, buffer->GetSpecification().Height };
		}
	}
	void OpenGLCommandBuffer::EndPresentRender()
	{
		if (m_Type == COMMAND_BUFFER_TYPE_GRAPHICS && m_FrameBuffer != nullptr)
		{
			m_FrameBuffer->EndPresentRender(this);
		}
	}
	void OpenGLCommandBuffer::Begin(Ref<FrameBuffer>& buffer)
	{
		Begin();
		BeginRenderPass(buffer);
	}
	void OpenGLCommandBuffer::Begin()
	{
		m_FrameBuffer.reset();
	}
	void OpenGLCommandBuffer::End()
	{
		EndRenderPass();
	}
	void OpenGLCommandBuffer::Render(Ref<GraphicsPipeline>& pipeline, uint32_t pass, uint32_t instanceCount, uint32_t indexCount)
	{
		if (!m_FrameBuffer)
		{
			glDisable(GL_SCISSOR_TEST);
			glViewport(0, 0, Graphics::GetViewportWidth(), Graphics::GetViewportHeight());
		}
		std::static_pointer_cast<OpenGLGraphicsPipeline>(pipeline)->Render(this, m_FrameBuffer, pass, instanceCount, indexCount);
	}
	void OpenGLCommandBuffer::Compute(Ref<ComputePipeline>& pipeline, uint32_t pass, uint32_t x, uint32_t y, uint32_t z)
	{
		std::static_pointer_cast<OpenGLComputePipeline>(pipeline)->Compute(this, pass, x, y, z);
	}
}
