#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsCommon.h"
#include "GEngine/Graphics/GraphicsPipeline.h"
#include "GEngine/Compute/ComputePipeline.h"
#include "GEngine/Graphics/FrameBuffer.h"
#include "GEngine/Components/Camera/CameraComponent.h"
#include "GEngine/Components/Camera/EditorCamera.h"
#include <set>

namespace GEngine
{
	struct GraphicsViewport
	{
		uint32_t X = 0, Y = 0, Width = 0, Height = 0;
	};
	

	class GENGINE_API CommandBuffer
	{
	public:
		virtual ~CommandBuffer() = default;

		virtual void Render(Ref<GraphicsPipeline>& pipeline, uint32_t pass, uint32_t instanceCount = 1, uint32_t indexCount = 0) = 0;
		// only for vulkan for now
		virtual void SwitchToNextSubpass() = 0;

		virtual void Compute(Ref<ComputePipeline>& pipeline, uint32_t pass, uint32_t x, uint32_t y, uint32_t z) = 0;

		virtual CommandBufferType				GetType() { return m_Type; }

		virtual void Begin(Ref<FrameBuffer>& buffer) = 0;
		virtual void Begin() = 0;
		virtual void BeginRenderPass(Ref<FrameBuffer>& buffer) = 0;
		virtual void BeginRenderPass(Ref<FrameBuffer>& buffer, const Ref<RenderPass>& renderPass) = 0;
		virtual void SetViewport(const GraphicsViewport& viewport) = 0;
		const GraphicsViewport& GetViewport() const { return m_Viewport; }
		virtual void EndRenderPass() = 0;
		virtual void End() = 0;
	protected:
		virtual void BeginPresentRender(Ref<FrameBuffer>& buffer) = 0;
		virtual void EndPresentRender() = 0;
	protected:
		void CheckViewport(const GraphicsViewport& viewport, const Ref<FrameBuffer>& buffer) const
		{
			if (!buffer || m_Type != COMMAND_BUFFER_TYPE_GRAPHICS || !viewport.Width || !viewport.Height ||
				uint64_t(viewport.X) + viewport.Width > buffer->GetSpecification().Width ||
				uint64_t(viewport.Y) + viewport.Height > buffer->GetSpecification().Height)
				throw std::invalid_argument("Viewport requires an active graphics target and an in-bounds nonempty rectangle.");
		}
		GraphicsViewport m_Viewport;
		CommandBufferType				m_Type = COMMAND_BUFFER_TYPE_NONE;
	};
}


