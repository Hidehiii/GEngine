#include "GEpch.h"
#include "OpenGLCommandBuffer.h"
#include "OpenGLGraphicsPipeline.h"
#include "OpenGLComputePipeline.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Math/Math.h"

namespace GEngine
{
	OpenGLCommandBuffer::OpenGLCommandBuffer(CommandBufferType type)
	{
		m_Type = type;
	}
	Ref<OpenGLCommandBuffer> OpenGLCommandBuffer::Create(CommandBufferType type)
	{
		return CreateRef<OpenGLCommandBuffer>(type);
	}
	void OpenGLCommandBuffer::Begin(Ref<FrameBuffer>& buffer)
	{
		m_FrameBuffer = std::static_pointer_cast<OpenGLFrameBuffer>(buffer);
		if (m_Type == CommandBufferType::Graphics)
		{
			m_FrameBuffer->Begin(this);
		}
		Graphics::UpdateScreenUniform(Vector4{ buffer->GetWidth(), buffer->GetHeight(), 0.0f, 0.0f });
	}
	void OpenGLCommandBuffer::End()
	{
		if (m_Type == CommandBufferType::Graphics)
		{
			m_FrameBuffer->End(this);
		}
		
	}
	void OpenGLCommandBuffer::Render(Ref<Scene>& scene)
	{
	}
	void OpenGLCommandBuffer::Render(Ref<GraphicsPipeline>& pipeline, uint32_t instanceCount, uint32_t indexCount)
	{
		std::static_pointer_cast<OpenGLGraphicsPipeline>(pipeline)->Render(this, m_FrameBuffer, instanceCount, indexCount);
	}
	void OpenGLCommandBuffer::Compute(Ref<ComputePipeline>& pipeline, uint32_t x, uint32_t y, uint32_t z)
	{
		std::static_pointer_cast<OpenGLComputePipeline>(pipeline)->Compute(this, x, y, z);
	}
}