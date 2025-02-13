#include "GEpch.h"
#include "OpenGLCommandBuffer.h"

namespace GEngine
{
	OpenGLCommandBuffer::OpenGLCommandBuffer()
	{
	}
	void OpenGLCommandBuffer::Begin(Ref<FrameBuffer>& buffer, const Editor::EditorCamera& camera, std::vector<CommandBuffer*> waitBuffers)
	{
		m_FrameBuffer = buffer;
		m_WaitBuffers = waitBuffers;
		m_FrameBuffer->Begin(this);
	}
	void OpenGLCommandBuffer::Begin(Ref<FrameBuffer>& buffer, const Camera& camera, std::vector<CommandBuffer*> waitBuffers)
	{
		m_FrameBuffer = buffer;
		m_WaitBuffers = waitBuffers;
		m_FrameBuffer->Begin(this);
	}
	void OpenGLCommandBuffer::End()
	{
		m_FrameBuffer->End(this);
	}
	void OpenGLCommandBuffer::Render(Ref<Scene>& scene)
	{
	}
	void OpenGLCommandBuffer::Render(Ref<GraphicsPipeline>& pipeline)
	{
	}
	void OpenGLCommandBuffer::Compute(Ref<ComputePipeline>& pipeline)
	{
	}
}