#include "Hierarchy.h"
#include <imgui_internal.h>
#include <filesystem>


namespace GEngine
{
	static void DrawFloatControl(const std::string& label, float& val, float restVal = 0.0f, float speed = 0.1f, float size = 100.0f)
	{
		ImGui::Columns(1);
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, size);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();
		ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });
		float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, { 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
		{
			val = restVal;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::DragFloat("##X", &val, speed);
		ImGui::PopItemWidth();


		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
	}
	static void DrawVector2Control(const std::string& label, Vector2& val, Vector2 restVal = 0.0f, float speed = 0.1f, float size = 100.0f)
	{
		ImGui::Columns(1);
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, size);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();
		ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });
		float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, { 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
		{
			val.value.x = restVal.value.x;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::DragFloat("##X", &val.value.x, speed);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, { 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
		{
			val.value.y = restVal.value.y;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::DragFloat("##Y", &val.value.y, speed);
		ImGui::PopItemWidth();


		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
	}
	static void DrawVector3Control(const std::string& label, Vector3& val, Vector3 restVal = 0.0f, float speed = 0.1f, float size = 100.0f)
	{
		ImGui::Columns(1);
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, size);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();
		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });
		float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, { 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if(ImGui::Button("X", buttonSize))
		{
			val.value.x = restVal.value.x;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::DragFloat("##X", &val.value.x, speed);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, { 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
		{
			val.value.y = restVal.value.y;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::DragFloat("##Y", &val.value.y, speed);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, { 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
		{
			val.value.z = restVal.value.z;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::DragFloat("##Z", &val.value.z, speed);
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
	}

	template <typename T>
	static void DrawAddComponentOption(const std::string& label, GameObject& gameObject)
	{
		if (ImGui::MenuItem(label.c_str()))
		{
			if (gameObject.HasComponent<T>() == false)
			{
				gameObject.AddComponent<T>();
			}
			ImGui::CloseCurrentPopup();
		}
	}
	static void DrawAddComponentButton(GameObject gameObject)
	{
		// 按钮显示在中间
		ImGui::Separator();
		ImGui::NewLine();
		ImVec2 contentRegionAvailable = ImGui::GetWindowContentRegionMax();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 20, 4 });
		float lineMiddle = GImGui->Font->FontSize * 2 + GImGui->Style.FramePadding.x;
		ImGui::SameLine(contentRegionAvailable.x * 0.5 - lineMiddle);
		if (ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup("AddComponent");
		}
		ImGui::PopStyleVar();
		if (ImGui::BeginPopup("AddComponent"))
		{
			DrawAddComponentOption<ImageRenderer>("Image Renderer", gameObject);
			DrawAddComponentOption<CircleRenderer>("Circle Renderer", gameObject);
			DrawAddComponentOption<Camera>("Camera", gameObject);
			DrawAddComponentOption<RigidBody2D>("RigidBody 2D", gameObject);
			DrawAddComponentOption<BoxCollider2D>("Box Collider 2D", gameObject);
			DrawAddComponentOption<CircleCollider2D>("Circle Collider 2D", gameObject);
			DrawAddComponentOption<MeshFilter>("Mesh Filter", gameObject);
			DrawAddComponentOption<MeshRenderer>("Mesh Renderer", gameObject);
			ImGui::EndPopup();
		}
	}

	template <typename T, typename UIFunc>
	static void DrawComponent(const std::string& name, GameObject gameObject, bool removeAble, UIFunc uiFunction)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Framed;
		if (gameObject.HasComponent<T>())
		{
			auto& component = gameObject.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (ImGui::Button("+", { lineHeight, lineHeight }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}
			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (removeAble)
				{
					if (ImGui::MenuItem("Remove Component"))
					{
						removeComponent = true;
					}
				}
					
				ImGui::EndPopup();
			}
			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}
			ImGui::PopStyleVar();
			ImGui::NewLine();
			if (removeComponent)
			{
				gameObject.RemoveComponent<T>();
			}
		}
	}


	Hierarchy::Hierarchy(const Ref<Scene> scene)
	{
		SetContext(scene);
	}
	void Hierarchy::SetContext(const Ref<Scene> scene)
	{
		m_Context = scene;
		m_SelectionContext = {};
	}
	void Hierarchy::DrawComponents(GameObject gameObject)
	{
		if (gameObject.HasComponent<Name>())
		{
			auto& n = gameObject.GetComponent<Name>();
			char buf[256] = { 0 };
			memset(buf, 0, sizeof(buf));
			strcpy_s(buf, sizeof(buf), n.m_Name.c_str());
			if (ImGui::InputText("##Name", buf, sizeof(buf)))
			{
				n.m_Name = std::string(buf);
			}
		}

		DrawComponent<Attribute>("Attribute", gameObject, false, [](auto& component)
			{
				ImGui::Checkbox("Active", &component.m_IsActive);
			}
		);
		

		DrawComponent<Transform>("Transform", gameObject, false, [](auto& component)
			{
				DrawVector3Control("Position", component.m_Position);
				Vector3 r = Math::Degrees(component.GetEulerAngles());
				DrawVector3Control("Rotation", r); 
				component.SetEulerAngle(r);
				DrawVector3Control("Scale", component.m_Scale, 1.0f);
			}
		);
		
		DrawComponent<Camera>("Camera", gameObject, true, [](auto& component)
			{
				ImGui::Checkbox("Is main camera", &component.isPrimary);


				std::string currentType = component.GetCameraTypeString(component.GetCameraType());
				if (ImGui::BeginCombo("Camera type", (const char*)currentType.c_str()))
				{
					bool isSelected = currentType == Camera::GetCameraTypeString(CameraType::OrthoGraphic);
					if (ImGui::Selectable((const char*)Camera::GetCameraTypeString(CameraType::OrthoGraphic).c_str(), isSelected))
					{
						component.SetCameraType(CameraType::OrthoGraphic);
					}
					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
					isSelected = currentType == Camera::GetCameraTypeString(CameraType::Perspective);
					if (ImGui::Selectable((const char*)Camera::GetCameraTypeString(CameraType::Perspective).c_str(), isSelected))
					{
						component.SetCameraType(CameraType::Perspective);
					}
					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				if (component.GetCameraType() == CameraType::OrthoGraphic)
				{
					DrawFloatControl("Size", component.m_OrthoGraphicSize);
					DrawFloatControl("Near clip", component.m_OrthoGraphicNear);
					DrawFloatControl("Far clip", component.m_OrthoGraphicFar);
				}

				if (component.GetCameraType() == CameraType::Perspective)
				{
					DrawFloatControl("FOV", component.m_PerspectiveFOV, 45.0f, 0.05f);
					DrawFloatControl("Near clip", component.m_PerspectiveNear);
					DrawFloatControl("Far clip", component.m_PerspectiveFar);
				}
			}
		);

		DrawComponent<ImageRenderer>("Image Renderer", gameObject, true, [](auto& component)
			{
				ImGui::ColorEdit4("Color", component.m_Color.ValuePtr());
				std::string texName = component.m_Texture ? component.m_Texture->GetPath() : "None";
				float size = 100.0f;
				ImGui::Columns(2);
				ImGui::SetColumnWidth(0, size);
				ImGui::Text("Texture");
				ImGui::NextColumn();
				float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
				ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
				buttonSize = { ImGui::CalcItemWidth(), lineHeight };
				ImGui::Button(texName.c_str(), buttonSize);
				// Recive Drag data
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						std::filesystem::path pathnam = (const wchar_t*)payload->Data;
						// Image data
						if (pathnam.extension() == ".png")
						{
							component.m_Texture = Texture2D::Create(pathnam.string());
						}

					}
					ImGui::EndDragDropTarget();
				}
				ImGui::SameLine();
				if (ImGui::Button("Reset"))
				{
					component.m_Texture = nullptr;
				}
				ImGui::Columns(1);
				DrawVector2Control("Tiling", component.m_Tiling, 1.0f);
			}
		);

		DrawComponent<CircleRenderer>("Circle Renderer", gameObject, true, [](auto& component)
			{
				ImGui::ColorEdit4("Color", component.m_Color.ValuePtr());
				std::string texName = component.m_Texture ? component.m_Texture->GetPath() : "None";
				float size = 100.0f;
				ImGui::Columns(2);
				ImGui::SetColumnWidth(0, size);
				ImGui::Text("Texture");
				ImGui::NextColumn();
				float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
				ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
				buttonSize = { ImGui::CalcItemWidth(), lineHeight };
				ImGui::Button(texName.c_str(), buttonSize);
				// Recive Drag data
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						std::filesystem::path pathnam = (const wchar_t*)payload->Data;
						// Image data
						if (pathnam.extension() == ".png")
						{
							component.m_Texture = Texture2D::Create(pathnam.string());
						}

					}
					ImGui::EndDragDropTarget();
				}
				ImGui::SameLine();
				if (ImGui::Button("Reset"))
				{
					component.m_Texture = nullptr;
				}
				ImGui::Columns(1);
				DrawFloatControl("Radius", component.m_Radius, 0.5f, 0.01f);
				DrawFloatControl("Thickness", component.m_Thickness, 0.1f, 0.01f);
				DrawFloatControl("Fade", component.m_Fade, 0.0f, 0.01f);
			}
		);

		DrawComponent<RigidBody2D>("RigidBody 2D", gameObject, true, [](auto& component)
			{

				std::string currentType = RigidBody2D::RigidBodyTypeToString(component.m_RigidBodyType);
				if (ImGui::BeginCombo("Body Type", (const char*)currentType.c_str()))
				{
					bool isSelected = currentType == RigidBody2D::RigidBodyTypeToString(RigidBody2D::RigidBodyType::Static);
					if (ImGui::Selectable((const char*)RigidBody2D::RigidBodyTypeToString(RigidBody2D::RigidBodyType::Static).c_str(), isSelected))
					{
						component.m_RigidBodyType = RigidBody2D::RigidBodyType::Static;
					}
					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
					isSelected = currentType == RigidBody2D::RigidBodyTypeToString(RigidBody2D::RigidBodyType::Dynamic);
					if (ImGui::Selectable((const char*)RigidBody2D::RigidBodyTypeToString(RigidBody2D::RigidBodyType::Dynamic).c_str(), isSelected))
					{
						component.m_RigidBodyType = RigidBody2D::RigidBodyType::Dynamic;
					}
					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
					isSelected = currentType == RigidBody2D::RigidBodyTypeToString(RigidBody2D::RigidBodyType::Kinematic);
					if (ImGui::Selectable((const char*)RigidBody2D::RigidBodyTypeToString(RigidBody2D::RigidBodyType::Kinematic).c_str(), isSelected))
					{
						component.m_RigidBodyType = RigidBody2D::RigidBodyType::Kinematic;
					}
					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				ImGui::Checkbox("Fixed Rotation", &component.m_FixedRotation);
			}
		);

		DrawComponent<BoxCollider2D>("Box Collider 2D", gameObject, true, [](auto& component)
			{
				DrawVector2Control("Offset", component.m_Offset);
				DrawVector2Control("Size", component.m_Size, { 1.0f, 1.0f },0.01f);
				DrawFloatControl("Rotation", component.m_Rotation);
				DrawFloatControl("Density", component.m_Density, 1.0f, 0.01f);
				DrawFloatControl("Friction", component.m_Friction, 0.5f, 0.01f);
				DrawFloatControl("Restitution", component.m_Restitution, 0.0f, 0.01f);
				DrawFloatControl("Restitution Threshold", component.m_RestitutionThreshold, 0.5f, 0.01f);
				ImGui::Checkbox("Is Trigger", &component.m_IsTrigger);
			}
		);

		DrawComponent<CircleCollider2D>("Circle Collider 2D", gameObject, true, [](auto& component)
			{
				DrawVector2Control("Offset", component.m_Offset);
				DrawFloatControl("Radius", component.m_Radius, 0.5f, 0.01f);
				DrawFloatControl("Density", component.m_Density, 1.0f, 0.01f);
				DrawFloatControl("Friction", component.m_Friction, 0.5f, 0.01f);
				DrawFloatControl("Restitution", component.m_Restitution, 0.0f, 0.01f);
				DrawFloatControl("Restitution Threshold", component.m_RestitutionThreshold, 0.5f, 0.01f);
				ImGui::Checkbox("Is Trigger", &component.m_IsTrigger);
			}
		);

		DrawComponent<MeshFilter>("Mesh Filter", gameObject, true, [](auto& component)
			{
				float size = 100.0f;
				ImGui::Columns(2);
				ImGui::SetColumnWidth(0, size);
				ImGui::Text("Mesh");
				ImGui::NextColumn();
				float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
				ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
				buttonSize = { ImGui::CalcItemWidth(), lineHeight };
				// Recive Drag data
				auto meshString = MeshLibrary::GetMeshNames();
				const char* currentMesh = component.GetMesh() ? component.GetMesh().m_Name.c_str() : "None";
				if (ImGui::BeginCombo("##1", currentMesh))
				{
					for (int i = 0; i < meshString.size(); i++)
					{
						bool isSelected = currentMesh == meshString[i].c_str();
						if (ImGui::Selectable(meshString[i].c_str(), isSelected))
						{
							component.SetMesh(MeshLibrary::GetMesh(meshString[i]));
						}
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
				ImGui::SameLine();
				if (ImGui::Button("Reset"))
				{
					component.SetMesh(Mesh());
				}
				ImGui::Columns(1);
			}
		);

		DrawComponent<MeshRenderer>("Mesh Renderer", gameObject, true, [](auto& component)
			{
				//ImGui::Text(component.m_Shader ? "None" : component.m_Shader->GetShaderName().c_str());
			}
		);
	}
	void Hierarchy::OnGuiRender()
	{


		ImGui::Begin("Scene Hierarchy");

		auto view = m_Context->m_Registry.view<Name>();
		for (auto entity : view)
		{
			GameObject g = m_Context->m_Registry.get<Name>(entity).m_GameObject;
			auto obj = g.GetComponent<Name>();
			ImGuiTreeNodeFlags flags = (((m_SelectionContext == g) ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth);
			bool isOpen =ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)g, flags, obj.m_Name.c_str());
			if (ImGui::IsItemClicked())
			{
				m_SelectionContext = g;
			}
			// Right click on object
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Delete gameobject"))
				{
					m_Context->DestroyGameObject(g);
					if (g == m_SelectionContext)
					{
						m_SelectionContext = {};
					}
				}
				ImGui::EndPopup();
			}
			if (isOpen)
			{
				ImGuiTreeNodeFlags flags = (((m_SelectionContext == g) ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth);
				if (ImGui::TreeNodeEx((void*)7123712, flags, obj.m_Name.c_str()))
				{
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
		}

		if (Input::IsMouseButtonPressed(MouseCode::Button0) && ImGui::IsWindowHovered())
		{
			m_SelectionContext = {};
		}

		// Right click on blank space
		if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			
			if (ImGui::MenuItem("Create empty gameobject"))
			{
				m_Context->CreateGameObject("Empty GameObject");
			}
			ImGui::EndPopup();
		}

		

		ImGui::End();

		ImGui::Begin("Properties");
		
		if (m_SelectionContext)
		{
			DrawComponents(m_SelectionContext);

			DrawAddComponentButton(m_SelectionContext);
		}
		ImGui::End();
		
	}
}