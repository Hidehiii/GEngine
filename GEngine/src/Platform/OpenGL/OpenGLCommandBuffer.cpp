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
		
		if (m_Type == CommandBufferType::Graphics)
		{
			GE_CORE_ASSERT(buffer != nullptr, "graphics cmd must have frame buffer");
			m_FrameBuffer = std::static_pointer_cast<OpenGLFrameBuffer>(buffer);
			m_FrameBuffer->Begin(this);
		}

		if(buffer != nullptr)
			Graphics::UpdateScreenUniform(Vector4{ buffer->GetWidth(), buffer->GetHeight(), 0.0f, 0.0f });
	}
	void OpenGLCommandBuffer::End()
	{
		if (m_Type == CommandBufferType::Graphics)
		{
			m_FrameBuffer->End(this);
		}
		
	}
	void OpenGLCommandBuffer::Render(Ref<GraphicsPipeline>& pipeline, int pass, uint32_t instanceCount, uint32_t indexCount)
	{
		std::static_pointer_cast<OpenGLGraphicsPipeline>(pipeline)->Render(this, m_FrameBuffer, pass, instanceCount, indexCount);
	}
	void OpenGLCommandBuffer::Compute(Ref<ComputePipeline>& pipeline, int pass, uint32_t x, uint32_t y, uint32_t z)
	{
		std::static_pointer_cast<OpenGLComputePipeline>(pipeline)->Compute(this, pass, x, y, z);
	}
}