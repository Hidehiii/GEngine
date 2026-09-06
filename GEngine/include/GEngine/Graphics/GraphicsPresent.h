#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Events/ApplicationEvent.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Renderer/FrameContext.h"
#include <set>

namespace GEngine
{
	class CommandBuffer;
	class Scene;
	class GraphicsPipeline;
	class ComputePipeline;
	class GraphicsResource;

	class GENGINE_API GraphicsPresent
	{
	public:
		GraphicsPresent();
		virtual ~GraphicsPresent();
		virtual bool AquireImage() { return true; }
		virtual void Begin() {}
		virtual void End() {}
		virtual bool AcquireFrame(FrameContext& frameContext);
		virtual void BeginFrame(const FrameContext& frameContext);
		virtual void EndFrame(FrameContext& frameContext);
		virtual Ref<GraphicsResource> GetPresentationResource() const { return nullptr; }

		static Ref<CommandBuffer> GetCommandBuffer();
		static void Render(Ref<GraphicsPipeline>& pipeline, const int& pass, uint32_t instanceCount = 1, uint32_t indexCount = 0);
		static void Compute(Ref<ComputePipeline>& pipeline, const int& pass, uint32_t x, uint32_t y, uint32_t z);
	protected:
		friend class RenderSystem;

		static Scope<GraphicsPresent> Create();
		static void SetActivePresenter(GraphicsPresent* presenter);
	protected:
		std::vector<Ref<CommandBuffer>>	m_CommandBuffers;
		static GraphicsPresent*			s_ActivePresenter;
	};
}
