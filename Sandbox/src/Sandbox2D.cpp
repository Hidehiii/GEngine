#include "Sandbox2D.h"


Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D")
{

}

void Sandbox2D::OnAttach()
{
	m_EditorCamera = GEngine::Editor::EditorCamera(30.0f, 1.0f, 0.1f, 1000.0f);
	m_Camera = GEngine::Editor::EditorCamera(30.0f, 1.0f, 0.1f, 1000.0f);
	m_RayTracing.Init();

}

void Sandbox2D::OnDetach()
{
}

void Sandbox2D::OnUpdate()
{
	GE_PROFILE_FUNCTION();
	m_EditorCamera.OnUpdate();

	GEngine::Renderer2D::ResetStats();

	{
		GE_PROFILE_SCOPE("Render: OnUpdate");
		// temporary
		GEngine::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		GEngine::RenderCommand::Clear();

		GEngine::Renderer::BeginScene(m_Camera);

		m_RayTracing.OnUpdate(m_EditorCamera);

		GEngine::Renderer::EndScene();
		
	}



}

void Sandbox2D::OnGuiRender()
{
	GE_PROFILE_FUNCTION();
	

	
}

void Sandbox2D::OnEvent(GEngine::Event& e)
{
	m_EditorCamera.OnEvent(e);
}
