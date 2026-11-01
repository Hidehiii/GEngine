#pragma once

#include <GEngine.h>
#include <GEngine/Renderer/RenderPipeline.h>

namespace GEngine
{
	class ForwardRenderPipeline final : public RenderPipeline
	{
	public:
		ForwardRenderPipeline() = default;
		void Resize(uint32_t width, uint32_t height);
		RenderGraph::TargetHandle GetTarget() const { return m_Target; }

		Ref<StorageBuffer>& GetColor()
		{
			return m_Color;
		}

	protected:
		void Configure(RenderPipelineBuilder& builder) override;

	private:
		Ref<StorageBuffer> m_Color;
		RenderGraph::TargetHandle m_Target = RenderGraph::InvalidTarget;
		uint32_t m_Width = 512, m_Height = 512;
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
		Ref<RenderPipelinePass> m_LifecycleChecks;
		std::vector<CameraComponent> m_Cameras;
		uint64_t m_FrameCount = 0;
		uint64_t m_GenerationFrames = 0;
	};
}
