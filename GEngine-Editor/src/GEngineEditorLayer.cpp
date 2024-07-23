#include "GEngineEditorLayer.h"


namespace GEngine
{
	static std::filesystem::path s_ModelPath = "Resources\\Model";
	static std::filesystem::path s_ShaderPath_2D = "Assets\\Shaders\\2D";
	static std::filesystem::path s_ShaderPath_3D = "Assets\\Shaders\\3D";


	GEngineEditorLayer::GEngineEditorLayer()
		: Layer("GEngine Editor")
	{

	}

	void GEngineEditorLayer::OnAttach()
	{
		ImGui::SetCurrentContext(Application::Get().GetImGuiLayer()->GetContext());

		m_PlayButtonIcon = Texture2D::Create("Resources/Icons/ToolBar/playButtonIcon.png");
		m_PlayingButtonIcon = Texture2D::Create("Resources/Icons/ToolBar/playingButtonIcon.png");
		m_StopButtonIcon = Texture2D::Create("Resources/Icons/ToolBar/stopButtonIcon.png");
		m_PauseButtonIcon = Texture2D::Create("Resources/Icons/ToolBar/pauseButtonIcon.png");
		m_PausingButtonIcon = Texture2D::Create("Resources/Icons/ToolBar/pausingButtonIcon.png");

		m_PlayButtonIcon_Display = m_PlayButtonIcon;
		m_PauseButtonIcon_DisPlay = m_PauseButtonIcon;

		FrameBufferSpecification fspec;
		fspec.Attachments = { FrameBufferTextureFormat::RGBA8, FrameBufferTextureFormat::Depth };
		fspec.Width = 1280;
		fspec.Height = 720;
		m_SceneViewportFrameBuffer = FrameBuffer::Create(fspec);
		m_GameViewportFrameBuffer = FrameBuffer::Create(fspec);
		m_EditorCamera = Editor::EditorCamera(30.0f, 1.778f, 0.01f, 10000.0f);
		m_EditorScene = CreateRef<Scene>();
		m_ActiveScene = m_EditorScene;
		
		m_Hierarchy.SetContext(m_ActiveScene);


		// Read all the model files in the model directory when the editor is attached
		{
			for (auto& modelFile : std::filesystem::directory_iterator(s_ModelPath))
			{
				const auto& path = modelFile.path();
				std::string filenameString = path.filename().string();
				if (path.extension() == ".fbx" || path.extension() == ".FBX")
				{
					GE_TRACE("Model: {0} loading", path);
					m_ModelImporter.LoadMesh(path.string());
					GE_TRACE("Model: {0} loaded", path);
				}
			}
			GE_TRACE("Model Count: {0}", MeshLibrary::GetMeshNames().size());
		}
		// Read all shader files
		{
			for (auto& shaderFile : std::filesystem::directory_iterator(s_ShaderPath_2D))
			{
				const auto& path = shaderFile.path();
				std::string filenameString = path.filename().string();
				if (path.extension() == ".glsl" || path.extension() == ".GLSL")
				{
					GE_TRACE("Shader: {0} loading", path);
					ShaderLibrary::Load(path.string());
					GE_TRACE("Shader: {0} loaded", path);
				}
			}
			for (auto& shaderFile : std::filesystem::directory_iterator(s_ShaderPath_3D))
			{
				const auto& path = shaderFile.path();
				std::string filenameString = path.filename().string();
				if (path.extension() == ".glsl" || path.extension() == ".GLSL")
				{
					GE_TRACE("Shader: {0} loading", path);
					ShaderLibrary::Load(path.string());
					GE_TRACE("Shader: {0} loaded", path);
				}
			}
		}


		// test
		{
			class TestScript : public ScriptableObject
			{
			public:
				virtual void OnCollisionEnter2D(Ref<Physics2DContactInfo> info)
				{
					GE_TRACE("OnCollisionEnter2D: {0}", info->GetOtherGameObject().GetComponent<Attribute>().m_Name);
				}
			};
			auto testObj = m_ActiveScene->CreateGameObject("Test");
			testObj.AddComponent<NativeScript>().Bind<TestScript>();
		}
	}

	void GEngineEditorLayer::OnDetach()
	{
	}

	void GEngineEditorLayer::OnUpdate()
	{
		GE_PROFILE_FUNCTION();

		// Scene update ( logic and physics )
		{
			if (m_SceneState == EditorSceneState::Play)
			{
				m_ActiveScene->OnUpdate();
			}
		}
		Renderer2D::ResetStats();

		// Scene Viewport
		{			
			// If framebuffer is changed in OnGuiRender(), the screen will flash
			FrameBufferSpecification fbspsc = m_SceneViewportFrameBuffer->GetSpecification();
			if (m_SceneViewportSize.value.x > 0 && m_SceneViewportSize.value.y > 0 && (fbspsc.Width != m_SceneViewportSize.value.x || fbspsc.Height != m_SceneViewportSize.value.y))
			{
				m_SceneViewportFrameBuffer->Resize(m_SceneViewportSize);
			}

			m_SceneViewportFrameBuffer->Bind();			

			{
				GE_PROFILE_SCOPE("Render: EditorOnRender");
				// temporary
				RenderCommand::SetClearColor({ 0.3f, 0.3f, 0.3f, 1 });
				RenderCommand::Clear();


				if (m_SceneViewportFocused && m_SceneViewportHovered)
				{
					m_EditorCamera.OnUpdate();
				}


				Renderer::BeginScene(m_EditorCamera);
				OnOverlayRender();
				m_ActiveScene->OnRender(); 
				Renderer::EndScene();
			}
			m_SceneViewportFrameBuffer->Unbind();

		}


		// Game Viewport
		{
			
			// If framebuffer is changed in OnGuiRender(), the screen will flash
			FrameBufferSpecification fbspsc = m_GameViewportFrameBuffer->GetSpecification();
			if (m_GameViewportSize.value.x > 0 && m_GameViewportSize.value.y > 0 && (fbspsc.Width != m_GameViewportSize.value.x || fbspsc.Height != m_GameViewportSize.value.y))
			{
				m_GameViewportFrameBuffer->Resize(m_GameViewportSize);
			}
			m_GameViewportFrameBuffer->Bind();

			{
				GE_PROFILE_SCOPE("Render: OnRender");
				// temporary
				RenderCommand::SetClearColor({ 0.3f, 0.3f, 0.3f, 1 });
				RenderCommand::Clear();

				
				auto camera =  m_ActiveScene->MainCamera();
				if (camera.m_GameObject)
				{
					Renderer::BeginScene(camera); 
					m_ActiveScene->OnRender();
					Renderer::EndScene();
				}
			}
			m_GameViewportFrameBuffer->Unbind();
		}
	}

	void GEngineEditorLayer::OnGuiRender()
	{
		GE_PROFILE_FUNCTION();




		bool dockSpace = true;
		// If you strip some features of, this demo is pretty much equivalent to calling DockSpaceOverViewport()!
   // In most cases you should be able to just call DockSpaceOverViewport() and ignore all the code below!
   // In this specific demo, we are not using DockSpaceOverViewport() because:
   // - we allow the host window to be floating/moveable instead of filling the viewport (when opt_fullscreen == false)
   // - we allow the host window to have padding (when opt_padding == true)
   // - we have a local menu bar in the host window (vs. you could use BeginMainMenuBar() + DockSpaceOverViewport() in your code!)
   // TL;DR; this demo is more complicated than what you would normally use.
   // If we removed all the options we are showcasing, this demo would become:
   //     void ShowExampleAppDockSpace()
   //     {
   //         ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
   //     }

		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockSpace, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowMinSize.x = 350.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize = ImVec2(32.0f,32.0f);
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				// Disabling fullscreen would allow the window to be moved to the front of other windows,
				// which we can't undo at the moment without finer window depth/z control.
				if (ImGui::MenuItem("New", "Ctrl+N"))
				{
					NewScene();
				}
				if (ImGui::MenuItem("Open...", "Ctrl+O"))
				{
					OpenScene();
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Save", "Ctrl+S"))
				{
					SaveScene();
				}
				if (ImGui::MenuItem("Save as...", "Ctrl+Shift+S"))
				{
					SaveSceneAs();
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Exit")) 
				{
					Application::Get().Close();
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		
		// Play / Stop Button
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, -2.0f });
			ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, { 0.0f, 2.0f });
			ImGui::Begin("##ToolBar",nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			float size = ImGui::GetWindowHeight() - 1.0f;
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x * 0.5f - 1.5f * 1.5f * size);
			if (ImGui::ImageButton((ImTextureID)m_PlayButtonIcon_Display->GetRendererID(), { 1.5f * size, size }, { 0.0f, 1.0f }, { 1.0f, 0.0f }))
			{
				OnScenePlay();
			}
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x * 0.5f);
			if (ImGui::ImageButton((ImTextureID)m_PauseButtonIcon_DisPlay->GetRendererID(), { 1.5f * size, size }, { 0.0f, 1.0f }, { 1.0f, 0.0f }))
			{
				OnScenePause();
			}
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x * 0.5f + 1.5f * 1.5f * size);
			if (ImGui::ImageButton((ImTextureID)m_StopButtonIcon->GetRendererID(), { 1.5f * size, size }, { 0.0f, 1.0f }, { 1.0f, 0.0f }))
			{
				OnSceneStop();
			}
			ImGui::PopStyleVar(2);
			ImGui::End();
		}
		
		

		// Scene Viewport
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
			ImGui::Begin("Scene Viewport");
			
			// Set Viewport Focused and Hovered
			m_SceneViewportFocused = ImGui::IsWindowFocused();
			m_SceneViewportHovered = ImGui::IsWindowHovered();
			Application::Get().GetImGuiLayer()->SetBlockEvent(!m_SceneViewportFocused || !m_SceneViewportHovered);

			// Update viewport size
			Vector2 viewportPanelSize = { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y };
			if (m_SceneViewportSize != viewportPanelSize && viewportPanelSize.value.x > 0 && viewportPanelSize.value.y > 0)
			{
				m_SceneViewportSize = viewportPanelSize;
				m_EditorCamera.SetViewportSize(m_SceneViewportSize.value.x, m_SceneViewportSize.value.y);
			}
			uint32_t tex = m_SceneViewportFrameBuffer->GetColorAttachmentRendererID();
			ImGui::Image((void*)tex, ImVec2(m_SceneViewportSize.value.x, m_SceneViewportSize.value.y), { 0.0f, 1.0f }, { 1.0f, 0.0f });

			Vector2 windowRegionMin = { ImGui::GetWindowContentRegionMin().x, ImGui::GetWindowContentRegionMin().y };
			Vector2 windowRegionMax = { ImGui::GetWindowContentRegionMax().x, ImGui::GetWindowContentRegionMax().y };
			Vector2 viewportOffset = { ImGui::GetWindowPos().x, ImGui::GetWindowPos().y };
			m_SceneViewportBounds[0] = windowRegionMin + viewportOffset;
			m_SceneViewportBounds[1] = windowRegionMax + viewportOffset;


			// Gizmos
			GameObject selectedObj = m_Hierarchy.GetSelectedGameObject();
			if (selectedObj && m_GizmoOperationType != -1)
			{
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, (float)ImGui::GetWindowWidth(), (float)ImGui::GetWindowHeight());

				Matrix4x4 cameraProjection = m_EditorCamera.GetProjectionMatrix();
				Matrix4x4 cameraView = m_EditorCamera.GetViewMatrix();

				auto& tc = selectedObj.GetComponent<Transform>();
				Matrix4x4 objTransform = tc.GetModelMatrix();
				

				// Snap
				bool isSnap = Input::IsKeyPressed(KeyCode::LeftControl);
				// for Scale and Position
				float snapVal = 0.5f;
				// for Rotation
				if (m_GizmoOperationType == ImGuizmo::OPERATION::ROTATE)
				{
					snapVal = 15.0f;
				}
				float snapArr[3] = { snapVal, snapVal , snapVal };

				Matrix4x4 deltaMatrix = objTransform;


				ImGuizmo::Manipulate(cameraView.ValuePtr(), cameraProjection.ValuePtr(), (ImGuizmo::OPERATION)m_GizmoOperationType, (ImGuizmo::MODE)m_GizmoModeType, objTransform.ValuePtr(), deltaMatrix.ValuePtr(), isSnap ? snapArr : nullptr);

				if (ImGuizmo::IsUsing())
				{
					// obj mat
					Vector3 position, rotation, scale;
					Math::DecomposeTransformMatrix(objTransform, position, rotation, scale); 
					tc.m_Position = position;
					tc.SetEulerAngle(Math::Degrees(rotation));
					tc.m_Scale = scale;
					// delta mat
					/*Vector3 deltaPosition, deltaRotation, deltaScale;
					Math::DecomposeTransformMatrix(deltaMatrix, deltaPosition, deltaRotation, deltaScale);
					tc.m_Position += deltaPosition;
					tc.Rotate(Math::Degrees(deltaRotation));
					tc.m_Scale = deltaScale;*/
				}

			}


			// Recive Drag data
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					std::filesystem::path pathnam = (const wchar_t*)payload->Data;
					// Scene data
					if (pathnam.extension() == ".GEScene")
					{
						m_SceneFilePath = pathnam;
						OpenScene();
					}
					
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::End();
			ImGui::PopStyleVar();
		}

		// Game Viewport
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
			ImGui::Begin("Game Viewport");

			// Update viewport size
			Vector2 viewportPanelSize = { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y };
			if (viewportPanelSize.value.x > 0 && viewportPanelSize.value.y > 0)
			{
				m_GameViewportSize = viewportPanelSize;
				m_ActiveScene->MainCamera().SetViewportSize(m_GameViewportSize.value.x, m_GameViewportSize.value.y);
			}
			uint32_t tex = m_GameViewportFrameBuffer->GetColorAttachmentRendererID();
			ImGui::Image((void*)tex, ImVec2(m_GameViewportSize.value.x, m_GameViewportSize.value.y), { 0.0f, 1.0f }, { 1.0f, 0.0f });

			ImGui::End();
			ImGui::PopStyleVar();
		}

		// Hierarchy
		{
			m_Hierarchy.OnGuiRender();
		}

		// Content Browser
		{
			m_ContentBrowser.OnGuiRender();
		}

		// Scene Viewport
		{
			ImGui::Begin("Scene Viewport Properties");


			// Gizmo operation
			{
				const char* gizmoOperationTypeString[] = { "None", "Translate", "Rotate", "Scale" };
				const char* currentGizmoOperationType = gizmoOperationTypeString[m_GizmoOperationType + 1];
				if (ImGui::BeginCombo("Gizmo operation", currentGizmoOperationType))
				{
					int size = 4;
					if (m_GizmoModeType == 1)
					{
						size = 3;
					}
					for (int i = 0; i < size; i++)
					{
						bool isSelected = m_GizmoOperationType == i - 1;
						if (ImGui::Selectable(gizmoOperationTypeString[i], isSelected))
						{
							m_GizmoOperationType = i - 1;
						}
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
				
			}

			// Gizmo mode
			{
				const char* gizmoModeTypeString[] = { "Local", "Global" };
				const char* currentGizmoModeType = gizmoModeTypeString[m_GizmoModeType];
				if (ImGui::BeginCombo("Gizmo mode", currentGizmoModeType))
				{
					int size = 2;
					if (m_GizmoOperationType == 2)
					{
						size = 1;
					}
					for (int i = 0; i < size; i++)
					{
						bool isSelected = m_GizmoModeType == i;
						if (ImGui::Selectable(gizmoModeTypeString[i], isSelected))
						{
							m_GizmoModeType = i;
						}
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
			}

			ImGui::NewLine();
			ImGui::Separator();
			ImGui::Text("Frames : %llf", 1 / Time::GetDeltaTime());

			ImGui::NewLine();
			ImGui::Separator();
			ImGui::Text("Stats:");
			ImGui::Text("Triangles: %d", Renderer2D::GetStats().m_TriangleCount);
			ImGui::Text("Lines: %d", Renderer2D::GetStats().m_LineCount);
			ImGui::Text("Points: %d", Renderer2D::GetStats().m_PointCount);
			ImGui::Text("Draw Calls: %d", Renderer2D::GetStats().m_DrawCalls);
			ImGui::Text("Vertex Count: %d", Renderer2D::GetStats().GetTotalVertexCount());
			ImGui::Text("Index Count: %d", Renderer2D::GetStats().GetTotalIndexCount());

			

			ImGui::End();
		}



		ImGui::End();
	}

	void GEngineEditorLayer::OnEndFrame()
	{
		m_ActiveScene->OnEndFrame();
	}


	void GEngineEditorLayer::OnEvent(Event& e)
	{
		m_EditorCamera.OnEvent(e);
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(GE_BIND_EVENT_FN(GEngineEditorLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(GE_BIND_EVENT_FN(GEngineEditorLayer::OnMouseButtonPressed));
	}

	bool GEngineEditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		// Shortcut
		if (e.IsRepeat())
		{
			return false;
		}

		bool controlPressed = Input::IsKeyPressed(KeyCode::LeftControl) || Input::IsKeyPressed(KeyCode::RightControl);
		bool shiftPressed = Input::IsKeyPressed(KeyCode::LeftShift) || Input::IsKeyPressed(KeyCode::RightShift);

		switch (e.GetKeyCode())
		{
			case KeyCode::N:
			{
				if (controlPressed)
				{
					// ctrl + N
					NewScene();
				}
				break;
			}
			case KeyCode::O:
			{
				if (controlPressed)
				{
					// ctrl + O
					OpenDialogToOpenScene();
				}
				break;
			}
			case KeyCode::S:
			{
				if (controlPressed && shiftPressed)
				{
					// ctrl + shift + S
					SaveSceneAs();
				}
				else if (controlPressed)
				{
					// ctrl + S
					SaveScene();
				}
				break;
			}
			case KeyCode::D:
			{
				if (controlPressed)
				{
					// ctrl + D
					DulicateGameObject();
				}
				break;
			}
		default:break;
		}

		return false;
	}

	bool GEngineEditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		return false;
	}

	void GEngineEditorLayer::NewScene()
	{
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize((uint32_t)m_SceneViewportSize.value.x, (uint32_t)m_SceneViewportSize.value.y);
		m_Hierarchy.SetContext(m_ActiveScene);
		m_HoveredGameObject = {};
		m_SceneFilePath = "";
	}


	void GEngineEditorLayer::OpenScene()
	{
		
		if (m_SceneFilePath.empty() == false && m_SceneFilePath.extension() == ".GEScene")
		{
			std::string filePath = m_SceneFilePath.string();
			m_ActiveScene = CreateRef<Scene>();
			m_ActiveScene->OnViewportResize((uint32_t)m_SceneViewportSize.value.x, (uint32_t)m_SceneViewportSize.value.y);
			m_Hierarchy.SetContext(m_ActiveScene);
			m_HoveredGameObject = {};
			Serializer::Deserialize(filePath, m_ActiveScene);
		}
	}

	void GEngineEditorLayer::OpenDialogToOpenScene()
	{
		std::string filePath = FileDialogs::OpenFile("GEngine Scene (*.GEScene)\0*.GEScene\0");
		if (filePath.empty() == false)
		{
			m_SceneFilePath = filePath;
			OpenScene();
		}
	}

	void GEngineEditorLayer::SaveScene()
	{
		if (m_SceneFilePath.empty() == false)
		{
			Serializer::Serialize(m_SceneFilePath.string(), m_ActiveScene);
		}
		else
		{
			SaveSceneAs();
		}
	}

	void GEngineEditorLayer::SaveSceneAs()
	{
		std::string filePath = FileDialogs::SaveFile("GEngine Scene (*.GEScene)\0*.GEScene\0");
		if (filePath.empty() == false)
		{
			m_SceneFilePath = filePath;
			Serializer::Serialize(filePath, m_ActiveScene);
		}
	}

	void GEngineEditorLayer::OnOverlayRender()
	{
		// Grid
		{
			float dis = 500.0f;
			for(int i = -dis; i <= dis; i += 10)
			{
				Renderer2D::DrawLine(Vector3(-dis, 0.0f, i), Vector3(dis, 0.0f, i), Vector4(0.4f, 0.4f, 0.4f, 1.0f));
				Renderer2D::DrawLine(Vector3(i, 0.0f, -dis), Vector3(i, 0.0f, dis), Vector4(0.4f, 0.4f, 0.4f, 1.0f));
			}
		}
		// Camera View
		{
			auto camera = m_ActiveScene->MainCamera();
			if (camera.m_GameObject)
			{
				if (camera.GetCameraType() == CameraType::Perspective)
				{
					// order: NLB NLT NRT NRB FLB FLT FRT FRB
					Vector3* coords = camera.GetPerspectiveClipCoord();

					Renderer2D::DrawLine(coords[0], coords[1], Vector4(1.0f));
					Renderer2D::DrawLine(coords[1], coords[2], Vector4(1.0f));
					Renderer2D::DrawLine(coords[2], coords[3], Vector4(1.0f));
					Renderer2D::DrawLine(coords[3], coords[0], Vector4(1.0f));

					Renderer2D::DrawLine(coords[4], coords[5], Vector4(1.0f));
					Renderer2D::DrawLine(coords[5], coords[6], Vector4(1.0f));
					Renderer2D::DrawLine(coords[6], coords[7], Vector4(1.0f));
					Renderer2D::DrawLine(coords[7], coords[4], Vector4(1.0f));

					Renderer2D::DrawLine(coords[0], coords[4], Vector4(1.0f));
					Renderer2D::DrawLine(coords[1], coords[5], Vector4(1.0f));
					Renderer2D::DrawLine(coords[2], coords[6], Vector4(1.0f));
					Renderer2D::DrawLine(coords[3], coords[7], Vector4(1.0f));
				}
				else
				{
					// order: NLB NLT NRT NRB FLB FLT FRT FRB
					Vector3* coords = camera.GetOrthoGraphicClipCoord();

					Renderer2D::DrawLine(coords[0], coords[1], Vector4(1.0f));
					Renderer2D::DrawLine(coords[1], coords[2], Vector4(1.0f));
					Renderer2D::DrawLine(coords[2], coords[3], Vector4(1.0f));
					Renderer2D::DrawLine(coords[3], coords[0], Vector4(1.0f));

					Renderer2D::DrawLine(coords[4], coords[5], Vector4(1.0f));
					Renderer2D::DrawLine(coords[5], coords[6], Vector4(1.0f));
					Renderer2D::DrawLine(coords[6], coords[7], Vector4(1.0f));
					Renderer2D::DrawLine(coords[7], coords[4], Vector4(1.0f));

					Renderer2D::DrawLine(coords[0], coords[4], Vector4(1.0f));
					Renderer2D::DrawLine(coords[1], coords[5], Vector4(1.0f));
					Renderer2D::DrawLine(coords[2], coords[6], Vector4(1.0f));
					Renderer2D::DrawLine(coords[3], coords[7], Vector4(1.0f));
				}
			}
		}
		// Main light direction
		{
			auto mainLight = m_ActiveScene->MainDirectionalLight();
			if (mainLight.m_GameObject)
			{
				auto transform = mainLight.m_GameObject.GetComponent<Transform>();
				Vector3 position = transform.m_Position;
				Vector3 direction = transform.Forward();
				Renderer2D::DrawCircle(transform, 0.1f, Vector4(1.0f, 1.0f, 0.0f, 1.0f));
				Renderer2D::DrawLine(position, direction , 1.0f, Vector4(1.0f, 1.0f, 0.0f, 1.0f));
				Renderer2D::DrawLine(position - 0.1 * transform.Right(), direction, 1.0f, Vector4(1.0f, 1.0f, 0.0f, 1.0f));
				Renderer2D::DrawLine(position + 0.1 * transform.Right(), direction, 1.0f, Vector4(1.0f, 1.0f, 0.0f, 1.0f));
				Renderer2D::DrawLine(position + 0.1 * transform.Up(), direction, 1.0f, Vector4(1.0f, 1.0f, 0.0f, 1.0f));
				Renderer2D::DrawLine(position - 0.1 * transform.Up(), direction, 1.0f, Vector4(1.0f, 1.0f, 0.0f, 1.0f));
			}
		}
		GameObject gameObject = m_Hierarchy.GetSelectedGameObject();
		if (gameObject)
		{
			// visualize the circle collider 2D
			if (gameObject.HasComponent<CircleCollider2D>())
			{
				auto circleCollider = gameObject.GetComponent<CircleCollider2D>();
				auto transform = gameObject.GetComponent<Transform>();
				Vector3 position = transform.m_Position + Vector3(circleCollider.m_Offset, 0.0f);
				Vector3 scale = Vector3(circleCollider.m_Radius * 2.0f) * Math::Max(transform.m_Scale);
				transform = Transform(position, Vector3(0.0f), scale);
				Renderer2D::DrawCircle(transform, 0.5f,  Vector4(0.0f, 1.0f, 0.0f, 1.0f));
			}
			// visualize the box collider 2D
			if (gameObject.HasComponent<BoxCollider2D>())
			{
				auto boxCollider = gameObject.GetComponent<BoxCollider2D>();
				auto transform = gameObject.GetComponent<Transform>();
				// This transform of the collider is should be multiplied by the scale transform of the game object
				// Otherwise, the collider will not be scaled with the game object
				auto t = Math::Translate(Matrix4x4(1.0f), transform.m_Position)
					* Math::Rotate(Matrix4x4(1.0f), Math::Degrees(transform.GetEulerAngles().value.z), Vector3(0.0f, 0.0f, 1.0f))
					* Math::Translate(Matrix4x4(1.0f), Vector3(boxCollider.m_Offset, 0.0f))
					* Math::Rotate(Matrix4x4(1.0f), boxCollider.m_Rotation, Vector3(0.0f, 0.0f, 1.0f))
					* Math::Scale(Matrix4x4(1.0f), transform.m_Scale * 1.01f)
					* Math::Scale(Matrix4x4(1.0f), Vector3(boxCollider.m_Size, 1.0f));
				Renderer2D::SetLineWidth(0.065f);
				Renderer2D::DrawRect(t, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
			}
		}
	}

	void GEngineEditorLayer::OnScenePlay()
	{
		if (m_SceneState == EditorSceneState::Play || m_SceneState == EditorSceneState::Pause)
		{
			OnSceneStop();
			return;
		}
		if (m_SceneState == EditorSceneState::Edit)
		{
			//TOOO: copy scene
			m_EditorScene = m_ActiveScene;
			m_ActiveScene = Scene::Copy(m_ActiveScene);
			m_ActiveScene->OnAwake();
			m_ActiveScene->OnStart();
			m_Hierarchy.SetContext(m_ActiveScene);
		}
		m_SceneState = EditorSceneState::Play;
		m_PlayButtonIcon_Display = m_PlayingButtonIcon;
		m_PauseButtonIcon_DisPlay = m_PauseButtonIcon;
	}

	void GEngineEditorLayer::OnSceneStop()
	{
		if (m_SceneState == EditorSceneState::Play || m_SceneState == EditorSceneState::Pause)
		{
			m_ActiveScene = m_EditorScene;
			m_Hierarchy.SetContext(m_ActiveScene);
		}
		m_SceneState = EditorSceneState::Edit;
		m_PlayButtonIcon_Display = m_PlayButtonIcon;
		m_PauseButtonIcon_DisPlay = m_PauseButtonIcon;
	}

	void GEngineEditorLayer::OnScenePause()
	{
		if (m_SceneState == EditorSceneState::Play)
		{
			m_SceneState = EditorSceneState::Pause;
			m_PauseButtonIcon_DisPlay = m_PausingButtonIcon;
		}
		else if (m_SceneState == EditorSceneState::Pause)
		{
			m_SceneState = EditorSceneState::Play;
			m_PauseButtonIcon_DisPlay = m_PauseButtonIcon;
		}
	}

	void GEngineEditorLayer::DulicateGameObject()
	{
		if (m_SceneState == EditorSceneState::Edit)
		{
			if (m_Hierarchy.GetSelectedGameObject())
			{
				m_ActiveScene->DuplicateGameObject(m_Hierarchy.GetSelectedGameObject());
			}
		}
	}
}
