#pragma once
#include "GEngine/Renderer/CommandBuffer.h"
#include "Platform/OpenGL/OpenGLFrameBuffer.h"

namespace GEngine
{
	class GENGINE_API OpenGLCommandBuffer : public CommandBuffer
	{
	public:
		OpenGLCommandBuffer();
		virtual ~OpenGLCommandBuffer() = default;

		virtual void Begin(Ref<FrameBuffer>&buffer, const Editor::EditorCamera & camera) override;
		virtual void Begin(Ref<FrameBuffer>&buffer, const Camera & camera) override;

		virtual void End() override;

		virtual void Render(Ref<Scene>&scene) override;
		virtual void Render(Ref<GraphicsPipeline>&pipeline, uint32_t instanceCount = 1, uint32_t indexCount = 0) override;

		virtual void Compute(Ref<ComputePipeline>&pipeline, uint32_t x, uint32_t y, uint32_t z) override;
	private:
		Ref<OpenGLFrameBuffer>	m_FrameBuffer;
	};
}


