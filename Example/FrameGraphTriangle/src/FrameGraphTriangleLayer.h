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
		void OnUpdate() override;
		void OnRender() override;
		void OnPresent() override;
		void OnImGuiRender() override;

	private:
		uint64_t m_RenderCount = 0;
		Ref<GraphicsPipeline> m_Pipeline;
		Ref<GraphicsPipeline> m_ComputeFirstPipeline;
		Ref<ComputePipeline> m_ComputePipeline;
		Ref<ComputePipeline> m_ComputeFirstComputePipeline;
		Ref<StorageBuffer> m_Color;
		Ref<StorageBuffer> m_ComputeFirstColor;
		Ref<Texture2DArray> m_RangeArray;
		RenderGraph m_Graph;
		RenderGraph::TargetHandle m_Target = RenderGraph::InvalidTarget;
	};
}
