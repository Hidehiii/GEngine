#pragma once

#include <GEngine.h>
#include <GEngine/Renderer/RenderGraph.h>

namespace GEngine
{
	// A small visual smoke test for the portable RenderSystem/RenderGraph path.
	// No backend header or native resource handle is used by this layer.
	class FrameGraphTriangleLayer final : public Layer
	{
	public:
		FrameGraphTriangleLayer();
		void OnAttach() override;
		void OnRender() override;
		void OnPresent() override;

	private:
		uint64_t m_RenderCount = 0;
		Ref<GraphicsPipeline> m_Pipeline;
		Ref<ComputePipeline> m_ComputePipeline;
		Ref<StorageBuffer> m_Color;
		RenderGraph m_Graph;
		RenderGraph::TargetHandle m_Target = RenderGraph::InvalidTarget;
	};
}
