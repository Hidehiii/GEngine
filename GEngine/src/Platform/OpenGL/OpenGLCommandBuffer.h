#pragma once
#include "GEngine/Renderer/CommandBuffer.h"

namespace GEngine
{
	class GENGINE_API OpenGLCommandBuffer : public CommandBuffer
	{
	public:
		OpenGLCommandBuffer();
		virtual ~OpenGLCommandBuffer() = default;

		virtual void Begin(Ref<FrameBuffer>&buffer, const Editor::EditorCamera & camera, std::vector<CommandBuffer*> waitBuffers) override;
		virtual void Begin(Ref<FrameBuffer>&buffer, const Camera & camera, std::vector<CommandBuffer*> waitBuffers) override;

		virtual void End() override;

		virtual void Render(Ref<Scene>&scene) override;
		virtual void Render(Ref<GraphicsPipeline>&pipeline) override;

		virtual void Compute(Ref<ComputePipeline>&pipeline) override;
	private:
		Ref<FrameBuffer>	m_FrameBuffer;
	};
}


