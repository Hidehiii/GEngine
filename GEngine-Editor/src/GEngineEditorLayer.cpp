#include "GEngineEditorLayer.h"
#include <GEngine/Graphics/GraphicsPresent.h>
#include <GEngine/Utils/PlatformUtils.h>
#include <cstdlib>
#include <GEngine/Graphics/RenderDevice.h>
#include <imgui_internal.h>


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

        const auto topLeftUV = GUIUtils::GetUV0();
        const auto bottomRightUV = GUIUtils::GetUV1();
        // Match the attachment origin without changing scene or camera coordinates.
        m_PresentVertexData[0].UV = { topLeftUV.x, bottomRightUV.y };
        m_PresentVertexData[1].UV = bottomRightUV;
        m_PresentVertexData[2].UV = { bottomRightUV.x, topLeftUV.y };
        m_PresentVertexData[3].UV = topLeftUV;
        auto vertices = VertexBuffer::Create(m_PresentVertexData.data(), static_cast<uint32_t>(sizeof(PresentVertex) * m_PresentVertexData.size()));
        const uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };
        vertices->SetIndexBuffer(IndexBuffer::Create(indices, 6));
        m_PresentPipeline = GraphicsPipeline::Create(Material::Create("Assets/Shaders/EditorPresent.shader", "EditorPresent"), vertices);
        m_PreviewShader = Shader::Create("Assets/Shaders/EditorPreview.shader");

		m_PlayButtonIcon			= Texture2D::Create("Resources/Icons/ToolBar/playButtonIcon.png");
		m_PlayingButtonIcon			= Texture2D::Create("Resources/Icons/ToolBar/playingButtonIcon.png");
		m_StopButtonIcon			= Texture2D::Create("Resources/Icons/ToolBar/stopButtonIcon.png");
		m_PauseButtonIcon			= Texture2D::Create("Resources/Icons/ToolBar/pauseButtonIcon.png");
		m_PausingButtonIcon			= Texture2D::Create("Resources/Icons/ToolBar/pausingButtonIcon.png");

		m_PlayButtonIcon_Display	= m_PlayButtonIcon;
		m_PauseButtonIcon_DisPlay	= m_PauseButtonIcon;

		RenderPassSpecification			spec;
		spec.RenderTargets			= { FRAME_BUFFER_TEXTURE_FORMAT_RGBA8 };
		spec.DepthStencil			= FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8;
		Ref<RenderPass> pass = RenderPass::Create(spec);

		m_SceneViewportFrameBuffer		= FrameBuffer::Create(pass, 1080, 720);
		m_GameViewportFrameBuffer		= FrameBuffer::Create(pass, 1080, 720);
		m_EditorCamera					= Editor::EditorCamera(30.0f, 1.778f, 0.01f, 10000.0f);
		m_EditorScene					= CreateRef<Scene>();
		m_ActiveScene					= m_EditorScene;
		m_Hierarchy.SetContext(m_ActiveScene);


        auto preview = m_ActiveScene->CreateGameObject("Preview quad");
        preview.AddComponent<ImageRenderer>(Vector4(0.2f, 0.65f, 0.95f, 1.0f));
    }

	void GEngineEditorLayer::OnDetach()
	{
	}

    void GEngineEditorLayer::OnUpdate()
    {
        if (m_SceneState == EditorSceneState::Play)
            m_ActiveScene->OnUpdate();
        if (m_SceneViewportFocused && m_SceneViewportHovered)
            m_EditorCamera.OnUpdate();
        auto resize = [](Ref<FrameBuffer>& target, Vector2 size)
        {
            if (size.x > 0 && size.y > 0 &&
                (target->GetWidth() != static_cast<uint32_t>(size.x) || target->GetHeight() != static_cast<uint32_t>(size.y)))
                target = FrameBuffer::Create(target, static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y));
        };
        resize(m_SceneViewportFrameBuffer, m_SceneViewportSize);
        resize(m_GameViewportFrameBuffer, m_GameViewportSize);
        m_ActiveScene->OnViewportResize(static_cast<uint32_t>(std::max(1.0f, m_GameViewportSize.x)), static_cast<uint32_t>(std::max(1.0f, m_GameViewportSize.y)));
    }

    void GEngineEditorLayer::OnRender()
    {
        RenderPreview(m_SceneViewportFrameBuffer, m_EditorCamera.GetViewProjection());
        Matrix4x4 gameViewProjection(1.0f);
        if (auto camera = m_ActiveScene->MainCamera())
            gameViewProjection = camera->GetProjectionMatrix() * Math::Inverse(camera->m_GameObject.GetComponent<Transform>().GetModelMatrix());
        RenderPreview(m_GameViewportFrameBuffer, gameViewProjection);
    }

    void GEngineEditorLayer::RenderPreview(Ref<FrameBuffer>& target, const Matrix4x4& viewProjection)
    {
        struct PreviewVertex { Vector4 Position; Vector4 Color; };
        auto command = Graphics::GetGraphicsCommandBuffer();
        command->Begin(target);
        auto draw = [&](std::vector<PreviewVertex>& vertices, const std::vector<uint32_t>& indices)
        {
            if (vertices.empty() || indices.empty()) return;
            auto buffer = VertexBuffer::Create(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(PreviewVertex)));
            buffer->SetIndexBuffer(IndexBuffer::Create(indices.data(), static_cast<uint32_t>(indices.size())));
            auto pipeline = GraphicsPipeline::Create(Material::Create(m_PreviewShader, "EditorPreview"), buffer);
            command->Render(pipeline, 0);
        };
        for (auto object : m_ActiveScene->GetGameObjects<Transform, ImageRenderer>())
        {
            auto& image = object.GetComponent<ImageRenderer>();
            auto matrix = viewProjection * object.GetComponent<Transform>().GetModelMatrix();
            std::vector<PreviewVertex> vertices;
            for (auto position : { Vector4(-0.5f,-0.5f,0,1), Vector4(0.5f,-0.5f,0,1), Vector4(0.5f,0.5f,0,1), Vector4(-0.5f,0.5f,0,1) })
                vertices.push_back({matrix * position, image.m_Color});
            draw(vertices, {0,1,2,2,3,0});
        }
        for (auto object : m_ActiveScene->GetGameObjects<Transform, MeshFilter, MeshRenderer>())
        {
            auto& mesh = object.GetComponent<MeshFilter>().GetMesh();
            auto matrix = viewProjection * object.GetComponent<Transform>().GetModelMatrix();
            std::vector<PreviewVertex> vertices;
            for (auto& vertex : mesh.m_Vertices) vertices.push_back({matrix * vertex.m_Position, vertex.m_Color});
            draw(vertices, mesh.m_Indices);
        }
        command->End();
        Graphics::SubmitCommandBuffer(command);
    }

	void GEngineEditorLayer::OnImGuiRender()
	{

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
			if (!ImGui::DockBuilderGetNode(dockspace_id))
			{
				ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
				ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);
				ImGuiID center = dockspace_id;
				auto left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.18f, nullptr, &center);
				auto right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.25f, nullptr, &center);
				auto bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.25f, nullptr, &center);
				auto toolbar = ImGui::DockBuilderSplitNode(center, ImGuiDir_Up, 0.08f, nullptr, &center);
				ImGui::DockBuilderDockWindow("Scene Hierarchy", left);
				for (auto name : { "Properties", "Setting", "Scene Viewport Properties" }) ImGui::DockBuilderDockWindow(name, right);
				ImGui::DockBuilderDockWindow("Content Browser", bottom);
				ImGui::DockBuilderDockWindow("##ToolBar", toolbar);
				ImGui::DockBuilderDockWindow("Scene Viewport", center);
				ImGui::DockBuilderDockWindow("Game Viewport", center);
				ImGui::DockBuilderFinish(dockspace_id);
			}
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
			if (ImGui::ImageButton("Play", (ImTextureID)GUIUtils::GetTextureID(m_PlayButtonIcon_Display), { 1.5f * size, size }, { GUIUtils::GetUV0().x, GUIUtils::GetUV0().y }, { GUIUtils::GetUV1().x, GUIUtils::GetUV1().y }))
			{
				m_SceneStateFunction = GE_BIND_CLASS_FUNCTION_LAMBDA(GEngineEditorLayer::OnScenePlay);
			}
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x * 0.5f);
			if (ImGui::ImageButton("Pause", (ImTextureID)GUIUtils::GetTextureID(m_PauseButtonIcon_DisPlay), { 1.5f * size, size }, { GUIUtils::GetUV0().x, GUIUtils::GetUV0().y }, { GUIUtils::GetUV1().x, GUIUtils::GetUV1().y }))
			{
				m_SceneStateFunction = GE_BIND_CLASS_FUNCTION_LAMBDA(GEngineEditorLayer::OnScenePause);
			}
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x * 0.5f + 1.5f * 1.5f * size);
			if (ImGui::ImageButton("Stop", (ImTextureID)GUIUtils::GetTextureID(m_StopButtonIcon), { 1.5f * size, size }, { GUIUtils::GetUV0().x, GUIUtils::GetUV0().y }, { GUIUtils::GetUV1().x, GUIUtils::GetUV1().y }))
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
			ImGui::Image(GUIUtils::GetTextureID(m_SceneViewportFrameBuffer->GetRenderTarget(0)), ImVec2(m_SceneViewportSize.x, m_SceneViewportSize.y), { GUIUtils::GetUV0().x, GUIUtils::GetUV0().y }, { GUIUtils::GetUV1().x, GUIUtils::GetUV1().y });

			Vector2 windowRegionMin		= { ImGui::GetWindowContentRegionMin().x, ImGui::GetWindowContentRegionMin().y };
			Vector2 windowRegionMax		= { ImGui::GetWindowContentRegionMax().x, ImGui::GetWindowContentRegionMax().y };
			Vector2 viewportOffset		= { ImGui::GetWindowPos().x, ImGui::GetWindowPos().y };
			m_SceneViewportBounds[0]	= windowRegionMin + viewportOffset;
			m_SceneViewportBounds[1]	= windowRegionMax + viewportOffset;
            OnOverlayRender();


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
				bool isSnap					= Input::GetKeyPressed(KeyCode::KEY_LEFT_CONTROL);
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
			ImGui::Image(GUIUtils::GetTextureID(m_GameViewportFrameBuffer->GetRenderTarget(0)), ImVec2(m_GameViewportSize.x, m_GameViewportSize.y), { GUIUtils::GetUV0().x, GUIUtils::GetUV0().y }, { GUIUtils::GetUV1().x, GUIUtils::GetUV1().y });
			//ImGui::Image(GUIUtils::GetTextureID(m_GameViewportFrameBuffer->GetDepthStencil()), ImVec2(m_GameViewportSize.x, m_GameViewportSize.y), { GUIUtils::GetUV0().x, GUIUtils::GetUV0().y }, { GUIUtils::GetUV1().x, GUIUtils::GetUV1().y });

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
            ImGui::TextUnformatted("Preview: vertex colors; textured and lit materials are not supported.");

			ImGui::End();
		}

		// setting
		{
			ImGui::Begin("Setting");

            ImGui::Text("Graphics API: %u", Application::Get().GetConfig()->GetGraphicsAPI());
            ImGui::Text("Frames in flight: %u", Application::Get().GetConfig()->GetFramesInFlight());
            ImGui::TextUnformatted("Change GraphicsAPI and FramesInFlight in Config.ini, then restart.");
            static bool vsync = Application::Get().GetConfig()->GetVSync();
            if (ImGui::Checkbox("VSync", &vsync)) Application::Get().GetWindow().SetVSync(vsync);

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
		auto material = m_PresentPipeline->GetMaterial();
		auto texture = Application::Get().GetImGuiLayer()->GetImGuiImage();
		if (material->GetShader()->GetPropertyType("GE_PRESENT_IMGUI") == SHADER_PROPERTY_TYPE_SAMPLER_TEXTURE_2D)
			material->SetResource("GE_PRESENT_IMGUI", Graphics::GetRenderDevice().CreateTexture2DCombineSampler(texture, Sampler::GetDefaultSampler()));
		else
		{
			material->SetResource("GE_PRESENT_IMGUI", texture);
			material->SetResource("GE_PRESENT_IMGUI_Sampler", Sampler::GetDefaultSampler());
		}
		GraphicsPresent::Render(m_PresentPipeline, 0);
	}

	void GEngineEditorLayer::OnEndFrame()
	{
		m_ActiveScene->OnEndFrame();
		m_SceneStateFunction();
		m_SceneStateFunction = []() {};
		static uint64_t frames = 0;
		++frames;
		const char* limit = std::getenv("GENGINE_EDITOR_FRAME_LIMIT");
		if (limit && std::strtoull(limit, nullptr, 10) > 0 && frames >= std::strtoull(limit, nullptr, 10))
		{
			GE_INFO("Editor completed {} frames; requesting normal shutdown.", frames);
			Application::Get().Close();
		}

	}


	void GEngineEditorLayer::OnEvent(Event& e)
	{
		m_EditorCamera.OnEvent(e);
		EventDispatcher::Dispatch<KeyPressedEvent>(e, GE_BIND_CLASS_FUNCTION_LAMBDA(GEngineEditorLayer::OnKeyPressed));
		EventDispatcher::Dispatch<MouseButtonPressedEvent>(e, GE_BIND_CLASS_FUNCTION_LAMBDA(GEngineEditorLayer::OnMouseButtonPressed));
	}

	bool GEngineEditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		// Shortcut
		if (!Input::GetKeyDown(e.GetKeyCode()))
		{
			return false;
		}

		bool controlPressed		= Input::GetKeyPressed(KeyCode::KEY_LEFT_CONTROL) || Input::GetKeyPressed(KeyCode::KEY_RIGHT_CONTROL);
		bool shiftPressed		= Input::GetKeyPressed(KeyCode::KEY_LEFT_SHIFT) || Input::GetKeyPressed(KeyCode::KEY_RIGHT_SHIFT);

		switch (e.GetKeyCode())
		{
			case KeyCode::KEY_N:
			{
				if (controlPressed)
				{
					// ctrl + N
					NewScene();
				}
				break;
			}
			case KeyCode::KEY_O:
			{
				if (controlPressed)
				{
					// ctrl + O
					OpenDialogToOpenScene();
				}
				break;
			}
			case KeyCode::KEY_S:
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
			case KeyCode::KEY_D:
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
			if(std::filesystem::path(filePath).extension() != ".GEScene")
			{ 
				filePath += ".GEScene";
			}
			m_SceneFilePath		= filePath;
			Serializer::Serialize(filePath, m_ActiveScene);
		}
	}

    void GEngineEditorLayer::OnOverlayRender()
    {
        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(m_SceneViewportBounds[0].x, m_SceneViewportBounds[0].y,
            m_SceneViewportSize.x, m_SceneViewportSize.y);
        auto view = m_EditorCamera.GetViewMatrix();
        auto projection = m_EditorCamera.GetProjectionMatrix();
        Matrix4x4 identity(1.0f);
        ImGuizmo::DrawGrid(Math::ValuePtr(view), Math::ValuePtr(projection), Math::ValuePtr(identity), 100.0f);
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
