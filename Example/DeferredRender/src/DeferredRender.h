#pragma once
#include <GEngine.h>

namespace GEngine
{
	struct PresentVertex
	{
		Vector4 Position;
		Vector2 UV;
	};

	class DeferredRender :public Layer
	{
	public:
		DeferredRender();
		virtual ~DeferredRender() = default;

		virtual void OnAttach() override;
		virtual void OnPresent() override;
		virtual void OnRender() override;

		void OnUpdate() override;
		virtual void OnImGuiRender() override;
		void OnEvent(Event & e) override;
	private:
		Editor::EditorCamera m_EditorCamera;

		Ref<FrameBuffer> m_GBuffer;
		Ref<RenderPass>	 m_BuildGBufferPass;

		Ref<FrameBuffer> m_LightingBuffer;
		Ref<RenderPass> m_LightingPass;

		Ref<GraphicsPipeline> m_CubePipeline;	
		Ref<GraphicsPipeline> m_SpherePipeline;
		Ref<GraphicsPipeline> m_CylinderPipeline;
		Ref<GraphicsPipeline> m_MonkeyPipeline;

		Ref<GraphicsPipeline> m_LightingPipeline;

		Ref<GraphicsPipeline> m_PresentPipeline;

		Ref<Texture2D> m_AlbedoTexture;
		Ref<Texture2D> m_RoughnessTexture;
		Ref<Texture2D> m_MetallicTexture;


		ModelImporter m_ModelImporter;
	};
}


