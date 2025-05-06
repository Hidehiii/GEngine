#pragma once
#include <GEngine.h>

namespace GEngine
{
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

		Ref<RenderPass> m_LightingPass;

		ModelImporter m_ModelImporter;
	};
}


