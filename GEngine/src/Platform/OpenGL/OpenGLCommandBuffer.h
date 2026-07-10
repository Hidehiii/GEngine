#pragma once
#include "GEngine/Graphics/CommandBuffer.h"
#include "Platform/OpenGL/OpenGLFrameBuffer.h"

namespace GEngine
{
	class GENGINE_API OpenGLCommandBuffer : public CommandBuffer
	{
	public:
		OpenGLCommandBuffer(CommandBufferType type);
		virtual ~OpenGLCommandBuffer() = default;

		virtual void Begin(Ref<FrameBuffer>& buffer) override;
		virtual void Begin() override;
		virtual void End() override;

		virtual void Render(Ref<GraphicsPipeline>&pipeline, uint32_t pass, uint32_t instanceCount = 1, uint32_t indexCount = 0) override;

		virtual void SwitchToNextSubpass() override { GE_CORE_ASSERT(false, "This command is only for vulkan!"); }

		virtual void Compute(Ref<ComputePipeline>&pipeline, uint32_t pass, uint32_t x, uint32_t y, uint32_t z) override;


		static Ref<OpenGLCommandBuffer>	Create(CommandBufferType type);
		
		bool operator==(const OpenGLCommandBuffer& other) const
		{
			return this == &other;
		}
	protected:
		virtual void BeginPresentRender(Ref<FrameBuffer>& buffer) override;
		virtual void EndPresentRender() override;
	private:
		Ref<OpenGLFrameBuffer>	m_FrameBuffer;
	};
}


