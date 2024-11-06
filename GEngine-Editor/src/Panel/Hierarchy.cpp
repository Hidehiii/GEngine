#include "Hierarchy.h"
#include <filesystem>
#include "Utils/GUIPainter.h"


namespace GEngine
{
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
			DrawAddComponentOption<ImageRenderer>	("Image Renderer",		gameObject);
			DrawAddComponentOption<CircleRenderer>	("Circle Renderer",		gameObject);
			DrawAddComponentOption<Camera>			("Camera",				gameObject);
			DrawAddComponentOption<RigidBody2D>		("RigidBody 2D",		gameObject);
			DrawAddComponentOption<BoxCollider2D>	("Box Collider 2D",		gameObject);
			DrawAddComponentOption<CircleCollider2D>("Circle Collider 2D",	gameObject);
			DrawAddComponentOption<MeshFilter>		("Mesh Filter",			gameObject);
			DrawAddComponentOption<MeshRenderer>	("Mesh Renderer",		gameObject);
			DrawAddComponentOption<DirectionalLight>("Directional Light",	gameObject);
			ImGui::EndPopup();
		}
	}

	template <typename T, typename UIFunc>
	static void DrawComponent(const std::string& name, GameObject gameObject, bool removeAble, UIFunc uiFunction)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Framed;
		if (gameObject.HasComponent<T>())
		{
			auto& component					= gameObject.GetComponent<T>();
			ImVec2 contentRegionAvailable	= ImGui::GetContentRegionAvail();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4, 4 });
			float lineHeight				= GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open						= ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
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
		m_Context			= scene;
		m_SelectionContext	= {};
	}
	void Hierarchy::DrawComponents(GameObject gameObject)
	{
		if (gameObject.HasComponent<Attribute>())
		{
			auto& attribute		= gameObject.GetComponent<Attribute>();
			char buf[256]		= { 0 };
			memset(buf, 0, sizeof(buf));
			strcpy_s(buf, sizeof(buf), attribute.m_Name.c_str());
			if (ImGui::InputText("##Name", buf, sizeof(buf)))
			{
				attribute.m_Name = std::string(buf);
			}
			ImGui::Checkbox("Active", &attribute.m_IsActive);
		}
		

		DrawComponent<Transform>("Transform", gameObject, false, [](auto& component)
			{
				GUI::Vector3Control("Position", component.m_Position);
				Vector3 r = Math::Degrees(Math::EulerAngles(component.m_Rotation));
				GUI::Vector3Control("Rotation", r);
				component.SetEulerAngle(r);
				GUI::Vector3Control("Scale", component.m_Scale, Vector3(1.0f));
			}
		);
		
		DrawComponent<Camera>("Camera", gameObject, true, [](auto& component)
			{
				GUI::Checkbox("Is main camera", component.isPrimary);


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
					GUI::FloatControl("Size", component.m_OrthoGraphicSize);
					GUI::FloatControl("Near clip", component.m_OrthoGraphicNear);
					GUI::FloatControl("Far clip", component.m_OrthoGraphicFar);
				}

				if (component.GetCameraType() == CameraType::Perspective)
				{
					GUI::FloatControl("FOV", component.m_PerspectiveFOV, 45.0f, 0.05f);
					GUI::FloatControl("Near clip", component.m_PerspectiveNear);
					GUI::FloatControl("Far clip", component.m_PerspectiveFar);
				}
			}
		);

		DrawComponent<DirectionalLight>("Directional Light", gameObject, true, [](auto& component)
			{
				GUI::Color4Control("Color", component.m_Color);
				GUI::Checkbox("Is Main", component.m_IsMain);
			}
		);

		DrawComponent<ImageRenderer>("Image Renderer", gameObject, true, [](auto& component)
			{
				GUI::Color4Control("Color", component.m_Color);
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
				GUI::Vector2Control("Tiling", component.m_Tiling, Vector2(1.0f));
			}
		);

		DrawComponent<CircleRenderer>("Circle Renderer", gameObject, true, [](auto& component)
			{
				GUI::Color4Control("Color", component.m_Color);
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
				GUI::FloatControl("Radius", component.m_Radius, 0.5f, 0.01f);
				GUI::FloatControl("Thickness", component.m_Thickness, 0.1f, 0.01f);
				GUI::FloatControl("Fade", component.m_Fade, 0.0f, 0.01f);
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
				GUI::Vector2Control("Offset", component.m_Offset);
				GUI::Vector2Control("Size", component.m_Size, { 1.0f, 1.0f },0.01f);
				GUI::FloatControl("Rotation", component.m_Rotation);
				GUI::FloatControl("Density", component.m_Density, 1.0f, 0.01f);
				GUI::FloatControl("Friction", component.m_Friction, 0.5f, 0.01f);
				GUI::FloatControl("Restitution", component.m_Restitution, 0.0f, 0.01f);
				GUI::FloatControl("Restitution Threshold", component.m_RestitutionThreshold, 0.5f, 0.01f);
				GUI::Checkbox("Is Trigger", component.m_IsTrigger);
			}
		);

		DrawComponent<CircleCollider2D>("Circle Collider 2D", gameObject, true, [](auto& component)
			{
				GUI::Vector2Control("Offset", component.m_Offset);
				GUI::FloatControl("Radius", component.m_Radius, 0.5f, 0.01f);
				GUI::FloatControl("Density", component.m_Density, 1.0f, 0.01f);
				GUI::FloatControl("Friction", component.m_Friction, 0.5f, 0.01f);
				GUI::FloatControl("Restitution", component.m_Restitution, 0.0f, 0.01f);
				GUI::FloatControl("Restitution Threshold", component.m_RestitutionThreshold, 0.5f, 0.01f);
				GUI::Checkbox("Is Trigger", component.m_IsTrigger);
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
				auto shaderString = ShaderLibrary::GetShaderNames();
				const char* currentShader = component.m_Material ? component.m_Material->GetShader()->GetShaderName().c_str() : "None";
				if (ImGui::BeginCombo("##2", currentShader))
				{
					for (int i = 0; i < shaderString.size(); i++)
					{
						bool isSelected = currentShader == shaderString[i].c_str();
						if (ImGui::Selectable(shaderString[i].c_str(), isSelected))
						{
							component.SetMaterial(Material::Create(ShaderLibrary::GetShader(shaderString[i])));
						}
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
				
				if (component.m_Material)
				{
					ImGui::Text(component.m_Material->GetName().c_str());
					auto& uniforms = component.m_Material->GetUniforms();
					for (auto& uniform : uniforms)
					{
						switch (uniform.Type)
						{
						case ShaderUniformType::Float:
						{
							float f = component.m_Material->GetFloat(uniform.Name);
							GUI::FloatControl(uniform.Name.c_str(), f);
							component.m_Material->SetFloat(uniform.Name, f);
							break;
						}
						case ShaderUniformType::Int:
						{
							break;
						}
						case ShaderUniformType::Vector:
						{
							Vector4 v = component.m_Material->GetVector(uniform.Name);
							GUI::Vector4Control(uniform.Name.c_str(), v);
							component.m_Material->SetVector(uniform.Name, v);
							break;
						}
						case ShaderUniformType::Color:
						{
							Vector4 v = component.m_Material->GetVector(uniform.Name);
							GUI::Color4Control(uniform.Name.c_str(), v);
							component.m_Material->SetVector(uniform.Name, v);
							break;
						}
						case ShaderUniformType::Mat3:
						{
							break;
						}
						case ShaderUniformType::Mat4 :
						{
							break;
						}
						default:
							break;
						}
					}
				}
			}
		);

	}
	void Hierarchy::OnGuiRender()
	{


		ImGui::Begin("Scene Hierarchy");

		auto view = m_Context->m_Registry.view<Attribute>();
		for (auto entity : view)
		{
			GameObject g = m_Context->m_Registry.get<Attribute>(entity).m_GameObject;
			auto obj = g.GetComponent<Attribute>();
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
				/*if (ImGui::TreeNodeEx((void*)7123712, flags, obj.m_Name.c_str()))
				{
					ImGui::TreePop();
				}*/
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