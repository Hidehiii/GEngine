#include "GEpch.h"
#include "OpenGLCommandBuffer.h"
#include "OpenGLGraphicsPipeline.h"
#include "OpenGLComputePipeline.h"

namespace GEngine
{
	OpenGLCommandBuffer::OpenGLCommandBuffer()
	{
		
	}
	void OpenGLCommandBuffer::Begin(Ref<FrameBuffer>& buffer, const Editor::EditorCamera& camera, CommandBufferType type)
	{
		m_Type = type;
		m_FrameBuffer = std::static_pointer_cast<OpenGLFrameBuffer>(buffer);
		m_FrameBuffer->Begin(this);
	}
	void OpenGLCommandBuffer::Begin(Ref<FrameBuffer>& buffer, const Camera& camera, CommandBufferType type)
	{
		m_Type = type;
		m_FrameBuffer = std::static_pointer_cast<OpenGLFrameBuffer>(buffer);
		m_FrameBuffer->Begin(this);
	}
	void OpenGLCommandBuffer::End()
	{
		m_FrameBuffer->End(this);
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