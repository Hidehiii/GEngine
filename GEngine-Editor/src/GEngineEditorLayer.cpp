#include "GEngineEditorLayer.h"


namespace GEngine
{
	static std::filesystem::path s_ModelPath		= "Resources\\Model";
	static std::filesystem::path s_ShaderPath_2D	= "Assets\\Shaders\\2D";
	static std::filesystem::path s_ShaderPath_3D	= "Assets\\Shaders\\3D";


	GEngineEditorLayer::GEngineEditorLayer()
		: Layer("GEngine Editor")
	{

	}

	void GEngineEditorLayer::OnAttach()
	{
		ImGui::SetCurrentContext(Application::Get().GetImGuiLayer()->GetContext());

		m_PresentPipeline			= Pipeline::Create(
			Material::Create(Shader::Create("Assets/Shaders/PresentImGui.glsl")),
			VertexBuffer::Create(sizeof(PresentVertex) * m_PresentVertexData.size())
		);
		m_PresentPipeline->GetVertexBuffer()->SetLayout({
			{ShaderDataType::float4,	"PositionOS"},
			{ShaderDataType::float2,	"UV"}
			});
		uint32_t*					presentIndices = new uint32_t[6];
		presentIndices[0]			= 0;
		presentIndices[1]			= 1;
		presentIndices[2]			= 2;

		presentIndices[3]			= 2;
		presentIndices[4]			= 3;
		presentIndices[5]			= 0;
		m_PresentPipeline->GetVertexBuffer()->SetIndexBuffer(IndexBuffer::Create(presentIndices, 6));
		delete[] presentIndices;
		m_PresentPipeline->GetVertexBuffer()->SetData(m_PresentVertexData.data(), sizeof(PresentVertex) * m_PresentVertexData.size());

		m_PlayButtonIcon			= Texture2D::Create("Resources/Icons/ToolBar/playButtonIcon.png");
		m_PlayingButtonIcon			= Texture2D::Create("Resources/Icons/ToolBar/playingButtonIcon.png");
		m_StopButtonIcon			= Texture2D::Create("Resources/Icons/ToolBar/stopButtonIcon.png");
		m_PauseButtonIcon			= Texture2D::Create("Resources/Icons/ToolBar/pauseButtonIcon.png");
		m_PausingButtonIcon			= Texture2D::Create("Resources/Icons/ToolBar/pausingButtonIcon.png");

		m_PlayButtonIcon_Display	= m_PlayButtonIcon;
		m_PauseButtonIcon_DisPlay	= m_PauseButtonIcon;

		FrameBufferSpecification		fspec;
		fspec.Attachments				= { FrameBufferTextureFormat::RGBA8, FrameBufferTextureFormat::DEPTH };
		fspec.Width						= 1280;
		fspec.Height					= 720;
		m_SceneViewportFrameBuffer		= FrameBuffer::Create(fspec);
		m_GameViewportFrameBuffer		= FrameBuffer::Create(fspec);
		m_EditorCamera					= Editor::EditorCamera(30.0f, 1.778f, 0.01f, 10000.0f);
		m_EditorScene					= CreateRef<Scene>();
		m_ActiveScene					= m_EditorScene;
		
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
				void OnStart()
				{
					GE_TRACE("OnStart");
					GE_TRACE(m_GameObject.GetComponent<Attribute>().m_Name);
				}
				void OnCollisionEnter2D(Ref<Physics2DContactInfo> info)
				{
					GE_TRACE("OnCollisionEnter2D: {0}", info->GetOtherGameObject().GetComponent<Attribute>().m_Name);
				}
				void OnPhysicsUpdate()
				{
					//GE_TRACE("OnPhysicsUpdate");
				}
				void OnUpdate()
				{
					//GE_TRACE("OnUpdate");
				}
			};
			class SubTestScript : public TestScript
			{
			public:
				int i = 10;
			};
			class TestScript2 : public ScriptableObject
			{
			public:
				void OnAwake()
				{
					AddComponent<SubTestScript>();
				}
				void OnStart()
				{
					GE_TRACE("OnStart");
					GetComponent<SubTestScript>().i = 20;
					GE_CRITICAL(GetComponent<SubTestScript>().i);
				}
			};
			auto testObj = m_ActiveScene->CreateGameObject("Test");
			
			//testObj.AddComponent<NativeScript>().Bind<TestScript>();
			//testObj.AddComponent<TestScript>();
			//testObj.AddComponent<SubTestScript>();
			testObj.AddComponent<TestScript2>();
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
			FrameBufferSpecification fbspec = m_SceneViewportFrameBuffer->GetSpecification();
			if (m_SceneViewportSize.x > 0 && m_SceneViewportSize.y > 0 && (fbspec.Width != m_SceneViewportSize.x || fbspec.Height != m_SceneViewportSize.y)
				|| Camera::AntiAliasingTypeToInt(m_EditorCamera.m_AntiAliasingType) != fbspec.Samples)
			{
				fbspec.Samples = Camera::AntiAliasingTypeToInt(m_EditorCamera.m_AntiAliasingType);
				fbspec.Width = m_SceneViewportSize.x;
				fbspec.Height = m_SceneViewportSize.y;
				m_SceneViewportFrameBuffer = FrameBuffer::Create(fbspec);
			}

			if (m_SceneViewportFocused && m_SceneViewportHovered)
			{
				m_EditorCamera.OnUpdate();
			}
		}


		// Game Viewport
		{
			// If framebuffer is changed in OnGuiRender(), the screen will flash
			FrameBufferSpecification fbspec = m_GameViewportFrameBuffer->GetSpecification();
			auto camera = m_ActiveScene->MainCamera();
			if (m_GameViewportSize.x > 0 && m_GameViewportSize.y > 0 && (fbspec.Width != m_GameViewportSize.x || fbspec.Height != m_GameViewportSize.y))
			{
				fbspec.Width	= m_GameViewportSize.x;
				fbspec.Height	= m_GameViewportSize.y;
				m_GameViewportFrameBuffer = FrameBuffer::Create(fbspec);
			}
			if ((camera && Camera::AntiAliasingTypeToInt(camera->m_AntiAliasingType) != fbspec.Samples))
			{
				fbspec.Samples = Camera::AntiAliasingTypeToInt(camera->m_AntiAliasingType);
				m_GameViewportFrameBuffer = FrameBuffer::Create(fbspec);
			}
		}
	}

	void GEngineEditorLayer::OnRender()
	{
		RenderCommand::BeginDrawCommand();
		m_SceneViewportFrameBuffer->Begin();
		{
			GE_PROFILE_SCOPE("Render: EditorOnRender");
			Renderer::BeginScene(m_EditorCamera);
			OnOverlayRender();
			m_ActiveScene->OnRender();
			Renderer::EndScene();
		}
		m_SceneViewportFrameBuffer->End();
		RenderCommand::EndDrawCommand();

		RenderCommand::BeginDrawCommand();
		m_GameViewportFrameBuffer->Begin();
		{
			GE_PROFILE_SCOPE("Render: OnRender");
			auto camera = m_ActiveScene->MainCamera();
			if (camera)
			{
				Renderer::BeginScene(*camera);
				m_ActiveScene->OnRender();
				Renderer::EndScene();
			}
		}
		m_GameViewportFrameBuffer->End();
		RenderCommand::EndDrawCommand();
	}

	void GEngineEditorLayer::OnImGuiRender()
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

		static bool opt_fullscreen					= true;
		static bool opt_padding						= false;
		static ImGuiDockNodeFlags dockspace_flags	= ImGuiDockNodeFlags_None;

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
		ImGuiIO& io				= ImGui::GetIO();
		ImGuiStyle& style		= ImGui::GetStyle();
		style.WindowMinSize.x	= 150.0f;
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
					OpenDialogToOpenScene();
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
			if (ImGui::ImageButton((ImTextureID)GUIUtils::GetTextureID(m_PlayButtonIcon_Display), { 1.5f * size, size }, { GUIUtils::GetUV0().x, GUIUtils::GetUV0().y }, { GUIUtils::GetUV1().x, GUIUtils::GetUV1().y }))
			{
				m_SceneStateFunction = GE_BIND_CLASS_FUNCTION_LAMBDA(GEngineEditorLayer::OnScenePlay);
			}
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x * 0.5f);
			if (ImGui::ImageButton((ImTextureID)GUIUtils::GetTextureID(m_PauseButtonIcon_DisPlay), { 1.5f * size, size }, { GUIUtils::GetUV0().x, GUIUtils::GetUV0().y }, { GUIUtils::GetUV1().x, GUIUtils::GetUV1().y }))
			{
				m_SceneStateFunction = GE_BIND_CLASS_FUNCTION_LAMBDA(GEngineEditorLayer::OnScenePause);
			}
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x * 0.5f + 1.5f * 1.5f * size);
			if (ImGui::ImageButton((ImTextureID)GUIUtils::GetTextureID(m_StopButtonIcon), { 1.5f * size, size }, { GUIUtils::GetUV0().x, GUIUtils::GetUV0().y }, { GUIUtils::GetUV1().x, GUIUtils::GetUV1().y }))
			{
				m_SceneStateFunction = GE_BIND_CLASS_FUNCTION_LAMBDA(GEngineEditorLayer::OnSceneStop);
			}
			ImGui::PopStyleVar(2);
			ImGui::End();
		}
		
		

		// Scene Viewport
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
			ImGui::Begin("Scene Viewport");
			
			// Set Viewport Focused and Hovered
			m_SceneViewportFocused		= ImGui::IsWindowFocused();
			m_SceneViewportHovered		= ImGui::IsWindowHovered();
			Application::Get().GetImGuiLayer()->SetBlockEvent(!m_SceneViewportFocused || !m_SceneViewportHovered);

			// Update viewport size
			Vector2 viewportPanelSize	= { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y };
			if (m_SceneViewportSize != viewportPanelSize && viewportPanelSize.x > 0 && viewportPanelSize.y > 0)
			{
				m_SceneViewportSize		= viewportPanelSize;
				m_EditorCamera.SetViewportSize(m_SceneViewportSize.x, m_SceneViewportSize.y);
			}
			ImGui::Image(GUIUtils::GetTextureID(m_SceneViewportFrameBuffer->GetColorAttachment(0)), ImVec2(m_SceneViewportSize.x, m_SceneViewportSize.y), { GUIUtils::GetUV0().x, GUIUtils::GetUV0().y }, { GUIUtils::GetUV1().x, GUIUtils::GetUV1().y });

			Vector2 windowRegionMin		= { ImGui::GetWindowContentRegionMin().x, ImGui::GetWindowContentRegionMin().y };
			Vector2 windowRegionMax		= { ImGui::GetWindowContentRegionMax().x, ImGui::GetWindowContentRegionMax().y };
			Vector2 viewportOffset		= { ImGui::GetWindowPos().x, ImGui::GetWindowPos().y };
			m_SceneViewportBounds[0]	= windowRegionMin + viewportOffset;
			m_SceneViewportBounds[1]	= windowRegionMax + viewportOffset;


			// Gizmos
			GameObject selectedObj = m_Hierarchy.GetSelectedGameObject();
			if (selectedObj && m_GizmoOperationType != -1)
			{
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, (float)ImGui::GetWindowWidth(), (float)ImGui::GetWindowHeight());

				Matrix4x4 cameraProjection	= m_EditorCamera.GetProjectionMatrix();
				Matrix4x4 cameraView		= m_EditorCamera.GetViewMatrix();

				auto& tc					= selectedObj.GetComponent<Transform>();
				Matrix4x4 objTransform		= tc.GetModelMatrix();
				

				// Snap
				bool isSnap					= Input::IsKeyPressed(KeyCode::LeftControl);
				// for Scale and Position
				float snapVal				= 0.5f;
				// for Rotation
				if (m_GizmoOperationType == ImGuizmo::OPERATION::ROTATE)
				{
					snapVal = 15.0f;
				}
				float snapArr[3]			= { snapVal, snapVal , snapVal };

				Matrix4x4 deltaMatrix		= objTransform;


				ImGuizmo::Manipulate(Math::ValuePtr(cameraView), Math::ValuePtr(cameraProjection), (ImGuizmo::OPERATION)m_GizmoOperationType, (ImGuizmo::MODE)m_GizmoModeType, Math::ValuePtr(objTransform), Math::ValuePtr(deltaMatrix), isSnap ? snapArr : nullptr);

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
			if (viewportPanelSize.x > 0 && viewportPanelSize.y > 0)
			{
				m_GameViewportSize	= viewportPanelSize;
				if (m_ActiveScene->MainCamera())
				{
					m_ActiveScene->MainCamera()->SetViewportSize(m_GameViewportSize.x, m_GameViewportSize.y);
				}
			}
			//ImGui::Image(GUIUtils::GetTextureID(m_GameViewportFrameBuffer->GetColorAttachment(0)), ImVec2(m_GameViewportSize.x, m_GameViewportSize.y), { GUIUtils::GetUV0().x, GUIUtils::GetUV0().y }, { GUIUtils::GetUV1().x, GUIUtils::GetUV1().y });
			ImGui::Image(GUIUtils::GetTextureID(m_GameViewportFrameBuffer->GetDepthAttachment()), ImVec2(m_GameViewportSize.x, m_GameViewportSize.y), { GUIUtils::GetUV0().x, GUIUtils::GetUV0().y }, { GUIUtils::GetUV1().x, GUIUtils::GetUV1().y });

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
				const char* gizmoOperationTypeString[]	= { "None", "Translate", "Rotate", "Scale" };
				const char* currentGizmoOperationType	= gizmoOperationTypeString[m_GizmoOperationType + 1];
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
				const char* gizmoModeTypeString[]	= { "Local", "Global" };
				const char* currentGizmoModeType	= gizmoModeTypeString[m_GizmoModeType];
				if (ImGui::BeginCombo("Gizmo mode", currentGizmoModeType))
				{
					int size = 2;
					if (m_GizmoOperationType == 2)
					{
						size = 1;
					}
					for (int i = 0; i < size; i++)
					{
						bool isSelected		= m_GizmoModeType == i;
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
			ImGui::Text("Triangles: %d",	Renderer2D::GetStats().m_TriangleCount);
			ImGui::Text("Lines: %d",		Renderer2D::GetStats().m_LineCount);
			ImGui::Text("Points: %d",		Renderer2D::GetStats().m_PointCount);
			ImGui::Text("Draw Calls: %d",	Renderer2D::GetStats().m_DrawCalls);
			ImGui::Text("Vertex Count: %d", Renderer2D::GetStats().GetTotalVertexCount());
			ImGui::Text("Index Count: %d",	Renderer2D::GetStats().GetTotalIndexCount());

			

			ImGui::End();
		}

		// setting
		{
			ImGui::Begin("Setting");

			const char* setRendererAPIString[] = { "OpenGL", "Vulkan" };
			if (ImGui::BeginCombo("Renderer API", setRendererAPIString[Application::Get().GetConfig()->m_RendererAPI - 1]))
			{
				int size = 2;
				for (int i = 0; i < size; i++)
				{
					bool isSelected = setRendererAPIString[i] == setRendererAPIString[Application::Get().GetConfig()->m_RendererAPI - 1];
					if (ImGui::Selectable(setRendererAPIString[i], isSelected))
					{
						if (setRendererAPIString[i] == "OpenGL")
						{
							Application::Get().GetConfig()->m_RendererAPI = 1;
						}
						else if (setRendererAPIString[i] == "Vulkan")
						{
							Application::Get().GetConfig()->m_RendererAPI = 2;
						}
					}
					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			ImGui::End();
		}


		ImGui::End();
	}

	void GEngineEditorLayer::OnLateUpdate()
	{
		if(m_SceneState == EditorSceneState::Play)
			m_ActiveScene->OnLateUpdate();
	}

	void GEngineEditorLayer::OnPresent()
	{
		m_PresentPipeline->GetMaterial()->SetTexture2D("GE_PRESENT_IMGUI", Application::Get().GetImGuiLayer()->GetImGuiImage());
		m_PresentPipeline->Render();
	}

	void GEngineEditorLayer::OnEndFrame()
	{
		m_ActiveScene->OnEndFrame();
		m_SceneStateFunction();
		m_SceneStateFunction = []() {};
	}


	void GEngineEditorLayer::OnEvent(Event& e)
	{
		m_EditorCamera.OnEvent(e);
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(GE_BIND_CLASS_FUNCTION_LAMBDA(GEngineEditorLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(GE_BIND_CLASS_FUNCTION_LAMBDA(GEngineEditorLayer::OnMouseButtonPressed));
	}

	bool GEngineEditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		// Shortcut
		if (e.IsRepeat())
		{
			return false;
		}

		bool controlPressed		= Input::IsKeyPressed(KeyCode::LeftControl) || Input::IsKeyPressed(KeyCode::RightControl);
		bool shiftPressed		= Input::IsKeyPressed(KeyCode::LeftShift) || Input::IsKeyPressed(KeyCode::RightShift);

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
					DuplicateGameObject();
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
		m_ActiveScene		= CreateRef<Scene>();
		m_ActiveScene->OnViewportResize((uint32_t)m_SceneViewportSize.x, (uint32_t)m_SceneViewportSize.y);
		m_Hierarchy.SetContext(m_ActiveScene);
		m_HoveredGameObject = {};
		m_SceneFilePath		= "";
	}


	void GEngineEditorLayer::OpenScene()
	{
		
		if (m_SceneFilePath.empty() == false && m_SceneFilePath.extension() == ".GEScene")
		{
			std::string filePath	= m_SceneFilePath.string();
			m_ActiveScene			= CreateRef<Scene>();
			m_ActiveScene->OnViewportResize((uint32_t)m_SceneViewportSize.x, (uint32_t)m_SceneViewportSize.y);
			m_Hierarchy.SetContext(m_ActiveScene);
			m_HoveredGameObject		= {};
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
		std::string filePath	= FileDialogs::SaveFile("GEngine Scene (*.GEScene)\0*.GEScene\0");
		if (filePath.empty() == false)
		{
			if(filePath.rfind(".GEScene") != 0)
			{ 
				filePath += ".GEScene";
			}
			m_SceneFilePath		= filePath;
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
			if (camera)
			{
				if (camera->GetCameraType() == CameraType::Perspective)
				{
					// order: NLB NLT NRT NRB FLB FLT FRT FRB
					Vector3* coords = camera->GetPerspectiveClipCoord();

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
					Vector3* coords = camera->GetOrthoGraphicClipCoord();

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
			if (mainLight)
			{
				auto transform = mainLight->m_GameObject.GetComponent<Transform>();
				Vector3 position = transform.m_Position;
				Vector3 direction = transform.Forward();
				Renderer2D::DrawCircle(transform, 0.1f, Vector4(1.0f, 1.0f, 0.0f, 1.0f));
				Renderer2D::DrawLine(position, direction , 1.0f, Vector4(1.0f, 1.0f, 0.0f, 1.0f));
				Renderer2D::DrawLine(position - 0.1f * transform.Right(),	direction, 1.0f, Vector4(1.0f, 1.0f, 0.0f, 1.0f));
				Renderer2D::DrawLine(position + 0.1f * transform.Right(),	direction, 1.0f, Vector4(1.0f, 1.0f, 0.0f, 1.0f));
				Renderer2D::DrawLine(position + 0.1f * transform.Up(),		direction, 1.0f, Vector4(1.0f, 1.0f, 0.0f, 1.0f));
				Renderer2D::DrawLine(position - 0.1f * transform.Up(),		direction, 1.0f, Vector4(1.0f, 1.0f, 0.0f, 1.0f));
			}
		}
		GameObject gameObject = m_Hierarchy.GetSelectedGameObject();
		if (gameObject)
		{
			// visualize the circle collider 2D
			if (gameObject.HasComponent<CircleCollider2D>())
			{
				auto circleCollider		= gameObject.GetComponent<CircleCollider2D>();
				auto transform			= gameObject.GetComponent<Transform>();
				Vector3 position		= transform.m_Position + Vector3(circleCollider.m_Offset, 0.0f);
				Vector3 scale			= Vector3(circleCollider.m_Radius * 2.0f) * Math::Max(transform.m_Scale);
				transform				= Transform(position, Vector3(0.0f), scale);
				Renderer2D::DrawCircle(transform, 0.5f,  Vector4(0.0f, 1.0f, 0.0f, 1.0f));
			}
			// visualize the box collider 2D
			if (gameObject.HasComponent<BoxCollider2D>())
			{
				auto boxCollider	= gameObject.GetComponent<BoxCollider2D>();
				auto transform		= gameObject.GetComponent<Transform>();
				// This transform of the collider is should be multiplied by the scale transform of the game object
				// Otherwise, the collider will not be scaled with the game object
				auto t				= Math::Translate(Matrix4x4(1.0f), transform.m_Position)
									* Math::Rotate(Matrix4x4(1.0f), Math::Degrees(transform.GetEulerAngle().z), Vector3(0.0f, 0.0f, 1.0f))
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
			m_EditorScene	= m_ActiveScene;
			m_ActiveScene	= Scene::Copy(m_ActiveScene);
			m_ActiveScene->OnAwake();
			m_ActiveScene->OnStart();
			m_Hierarchy.SetContext(m_ActiveScene);
		}
		m_SceneState				= EditorSceneState::Play;
		m_PlayButtonIcon_Display	= m_PlayingButtonIcon;
		m_PauseButtonIcon_DisPlay	= m_PauseButtonIcon;
	}

	void GEngineEditorLayer::OnSceneStop()
	{
		if (m_SceneState == EditorSceneState::Play || m_SceneState == EditorSceneState::Pause)
		{
			m_ActiveScene = m_EditorScene;
			m_Hierarchy.SetContext(m_ActiveScene);
		}
		m_SceneState				= EditorSceneState::Edit;
		m_PlayButtonIcon_Display	= m_PlayButtonIcon;
		m_PauseButtonIcon_DisPlay	= m_PauseButtonIcon;
	}

	void GEngineEditorLayer::OnScenePause()
	{
		if (m_SceneState == EditorSceneState::Play)
		{
			m_SceneState				= EditorSceneState::Pause;
			m_ActiveScene->OnPause();
			m_PauseButtonIcon_DisPlay	= m_PausingButtonIcon;
		}
		else if (m_SceneState == EditorSceneState::Pause)
		{
			m_SceneState				= EditorSceneState::Play;
			m_ActiveScene->OnResume();
			m_PauseButtonIcon_DisPlay	= m_PauseButtonIcon;
		}
	}

	void GEngineEditorLayer::DuplicateGameObject()
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
