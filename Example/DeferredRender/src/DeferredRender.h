#pragma once
#include <GEngine.h>
#include "DeferredRenderPipeline.h"

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
		Ref<DeferredRenderPipeline>	m_DeferredPipeline;

		Editor::EditorCamera m_EditorCamera;
	};
}


