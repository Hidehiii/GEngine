#pragma once
#include <GEngine.h>
#include "Panel/Hierarchy.h"
#include "Panel/ContentBrowser.h"
#include <filesystem>

namespace GEngine
{
	enum class EditorSceneState
	{
		Edit	= 0, 
		Play	= 1, 
		Pause	= 2
	};

	struct PresentVertex
	{
		Vector4	Pos;
		Vector2 UV;
	};

	class GEngineEditorLayer : public Layer
	{
	public:
		GEngineEditorLayer();
		virtual ~GEngineEditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate() override;
		virtual void OnRender() override;
		virtual void OnImGuiRender() override;
		virtual void OnLateUpdate() override;
		virtual void OnPresent() override;
		virtual void OnEndFrame() override;

		void OnEvent(GEngine::Event& e) override;
	private:
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		void NewScene();
		void OpenScene();
		void OpenDialogToOpenScene();
		void SaveScene();
		void SaveSceneAs();

		void OnOverlayRender();
		void OnScenePlay();
		void OnSceneStop();
		void OnScenePause();
		void DulicateGameObject();
	private:
		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_EditorScene;

		//--------------------------------------------
		// Editor
		Ref<FrameBuffer> m_SceneViewportFrameBuffer;
		Ref<FrameBuffer> m_GameViewportFrameBuffer;
		Vector2 m_SceneViewportSize = { 0.0f, 0.0f };
		Vector2 m_GameViewportSize = { 0.0f, 0.0f };
		bool m_SceneViewportFocused = false, m_SceneViewportHovered = false;
		bool m_GameViewportFocused = false, m_GameViewportHovered = false;
		Hierarchy m_Hierarchy;
		ContentBrowser m_ContentBrowser;
		Editor::EditorCamera m_EditorCamera;
		// Gizmo operation type
		// -1: None  0 : Translate  1 : Rotate  2 : Scale
		int m_GizmoOperationType = -1;
		// Gizmo mode type
		// 0 : Local  1 : Global
		int m_GizmoModeType = 0;
		Vector2 m_SceneViewportBounds[2];
		GameObject m_HoveredGameObject = {};
		std::filesystem::path m_SceneFilePath;

		Ref<Texture2D> m_PlayButtonIcon;
		Ref<Texture2D> m_PlayingButtonIcon;
		Ref<Texture2D> m_StopButtonIcon;
		Ref<Texture2D> m_PauseButtonIcon;
		Ref<Texture2D> m_PausingButtonIcon;

		Ref<Texture2D> m_PlayButtonIcon_Display;
		Ref<Texture2D> m_PauseButtonIcon_DisPlay;

		EditorSceneState m_SceneState = EditorSceneState::Edit;

		ModelImporter m_ModelImporter;

		// Present
		Ref<Pipeline>				m_PresentPipeline;
		std::vector<PresentVertex>	m_PresentVertexData = {
			{ {-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 0.0f} },
			{ { 1.0f, -1.0f, 0.0f, 1.0f}, {1.0f, 0.0f} },
			{ { 1.0f,  1.0f, 0.0f, 1.0f}, {1.0f, 1.0f} },
			{ {-1.0f,  1.0f, 0.0f, 1.0f}, {0.0f, 1.0f} }
		};
	private:
		std::function<void()> m_SceneStateFunction = [&]() {};
	};

}


