#pragma once

#include <GEngine.h>
#include <GEngine/Renderer/RenderPipeline.h>

namespace GEngine
{
	class ForwardRenderPipeline final : public RenderPipeline
	{
	public:
		ForwardRenderPipeline() = default;

		Ref<StorageBuffer>& GetColor()
		{
			return m_Color;
		}

	protected:
		void Configure(RenderPipelineBuilder& builder) override;

	private:
		Ref<StorageBuffer> m_Color;
		RenderGraph::TargetHandle m_Target = RenderGraph::InvalidTarget;
	};

	class RenderPipelineLayer final : public Layer
	{
	public:
		RenderPipelineLayer();
		void OnAttach() override;
		void OnRender() override;
		void OnPresent() override;

	private:
		ForwardRenderPipeline m_Pipeline;
		std::vector<CameraComponent> m_Cameras;
		uint64_t m_FrameCount = 0;
	};
}
