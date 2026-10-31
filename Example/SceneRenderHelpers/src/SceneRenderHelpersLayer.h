#pragma once

#include <GEngine.h>

namespace GEngine
{
	class SceneRenderHelpersLayer final : public Layer
	{
	public:
		SceneRenderHelpersLayer();
		void OnAttach() override;
		void OnUpdate() override;
		void OnPresent() override;
		void OnEndFrame() override;

	private:
		Ref<Scene> m_Scene;
		GameObject m_Image;
		GameObject m_Mesh;
		GameObject m_Light;
		uint64_t m_FrameCount = 0;
		bool m_FirstSyntheticResizeRequested = false;
		bool m_SecondSyntheticResizeRequested = false;
	};
}
