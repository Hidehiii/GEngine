#include "Sandbox2D.h"

using namespace GEngine;

static std::filesystem::path s_ModelPath = "Resources\\Model";
static std::filesystem::path s_ShaderPath_2D = "Assets\\Shaders\\2D";
static std::filesystem::path s_ShaderPath_3D = "Assets\\Shaders\\3D";

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D")
{

}

void Sandbox2D::OnAttach()
{
	/*m_EditorCamera = GEngine::Editor::EditorCamera(30.0f, 1.0f, 0.1f, 1000.0f);
	m_Camera = GEngine::Editor::EditorCamera(30.0f, 1.0f, 0.1f, 1000.0f);*/
	//m_RayTracing.Init();
	ImGui::SetCurrentContext(GEngine::Application::Get().GetImGuiLayer()->GetContext());
}

void Sandbox2D::OnDetach()
{
}

void Sandbox2D::OnUpdate()
{
	GE_PROFILE_FUNCTION();
	//m_EditorCamera.OnUpdate();

	//GEngine::Renderer2D::ResetStats();

	//{
	//	GE_PROFILE_SCOPE("Render: OnUpdate");
	//	// temporary
		GEngine::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		GEngine::RenderCommand::Clear();

	//	GEngine::Renderer::BeginScene(m_Camera);

	//	m_RayTracing.OnUpdate(m_EditorCamera);

	//	GEngine::Renderer::EndScene();
	//	
	//}


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
}

void Sandbox2D::OnGuiRender()
{
	GE_PROFILE_FUNCTION();
	ImGui::Begin("Profile");
	ImGui::Text("Frames : %llf", 1 / GEngine::Time::GetDeltaTime());
	ImGui::End();
}

void Sandbox2D::OnEvent(GEngine::Event& e)
{
	//m_EditorCamera.OnEvent(e);
}
