#include "GEpch.h"
#include "OpenGLCommandBuffer.h"

namespace GEngine
{
	OpenGLCommandBuffer::OpenGLCommandBuffer()
	{
	}
	void OpenGLCommandBuffer::Begin(Ref<FrameBuffer>& buffer, const Editor::EditorCamera& camera)
	{
	}
	void OpenGLCommandBuffer::Begin(Ref<FrameBuffer>& buffer, const Camera& camera)
	{
	}
	void OpenGLCommandBuffer::End()
	{
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