#include "Sandbox2D.h"

static const uint32_t s_MapWidth = 28;
static const char* s_MapTiles = 
"RRRRRRRRRRRRRRRRRRRRRRRRRRRR"
"RRRRRRRRRRRRRRRRRRRRRRRRRRRR"
"RRRRRRRRRRRRRRRRRRRRRRRRRRRR"
"RRRRRRRRRRRRRRRRRRRRRRRRRRRR"
"RRRRRRRRRRRRRRRRRRRRRRRRRRRR"
"RRRRRRRRRRRRRRRRRRRRRRRRRRRR"
"RRRRRWWWRRRRRRRRRRRRRRRRRRRR"
"RRRRRWWWRRRRRRRRRRRRRRRRRRRR"
"RRRRRWWWRRRRRRRRRRRRRRRRRRRR"
"RRRRRWWWRRRRRRRRRRRRRRRRRRRR"
"RRRRRWWWRRRRRRRRRRRRRRRRRRRR"
"RRRRRRRRRRRRRRRRRRRRRRRRRRRR"
"RRRRRRRRRRRRRRRRRRRRRRRRRRRR"
"RRRRRRRRRRRRRRRRRRRRRRRRRRRR"
"RRRRRRRRRRRRRRRRRRRRRRRRRRRR"
"RRRRRRRRRRRRRRRRRRRRRRRRRRRR"
"RRRRRRRRRRRRRRRRRRRRRRRRRRRR"
"RRRRRRRRRRRRRRRRRRRRRRRRRRRR"
"RRRRRRRRRRRRRRRRRRRRRRRRRRRR"
"RRRRRRRRRRRRRRRRRRRRRRRRRRRR"
"RRRRRRRRRRRRRRRRRRRRRRRRRRRR";

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D")
{
	m_CameraController.reset(new GEngine::OrthoGraphicCameraController(1280.0f / 720.0f, true));
}

void Sandbox2D::OnAttach()
{
	m_Texture_01 = GEngine::Texture2D::Create("Assets/Textures/01.png");
	m_Texture_02 = GEngine::Texture2D::Create("Assets/Textures/02.png");
	m_SpriteSheet = GEngine::Texture2D::Create("Assets/Game/Textures/tilemap_packed.png");
	m_TrafficLights = GEngine::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 3.0f, 2.0f }, { 16.0f, 16.0f });
	m_Forest = GEngine::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 18.0f, 4.0f }, { 16.0f, 16.0f }, { 3.0f, 3.0f});

	m_MapWidth = s_MapWidth;
	m_MapHeight = strlen(s_MapTiles) / s_MapWidth;

	m_TextureMap['R'] = GEngine::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 9.0f, 1.0f }, { 16.0f, 16.0f });
	m_TextureMap['W'] = GEngine::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 9.0f, 10.0f }, { 16.0f, 16.0f });

	m_CameraController->SetZoomLevel(5.5f);


	// Init here
	m_Particle.ColorBegin = { 254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f };
	m_Particle.ColorEnd = { 254 / 255.0f, 109 / 255.0f, 41 / 255.0f, 1.0f };
	m_Particle.SizeBegin = 0.5f, m_Particle.SizeVariation = 0.3f, m_Particle.SizeEnd = 0.0f;
	m_Particle.LifeTime = 1.0f;
	m_Particle.Velocity = { 0.0f, 0.0f };
	m_Particle.VelocityVariation = { 3.0f, 1.0f };
	m_Particle.Position = { 0.0f, 0.0f };

}

void Sandbox2D::OnDetach()
{
}

void Sandbox2D::OnUpdate()
{
	GE_PROFILE_FUNCTION();

	m_CameraController->OnUpdate();

	GEngine::Renderer2D::ResetStats();


	{
		GE_PROFILE_SCOPE("Render: OnUpdate");
		// temporary
		GEngine::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		GEngine::RenderCommand::Clear();

		/*GEngine::Renderer2D::BeginScene(m_CameraController->GetCamera());

		static GEngine::Vector3 rotation = 0.0f;
		rotation.value.z += GEngine::Time::GetDeltaTime() * 50.0f;
		GEngine::Renderer2D::DrawQuad({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, m_Color, m_Texture_01);
		GEngine::Renderer2D::DrawQuad({ 1.0f, 0.0f, 0.0f }, rotation, { 1.0f, 1.0f, 1.0f }, m_Color, m_Texture_02, { 2.0f, 2.0f});
		GEngine::Renderer2D::DrawQuad({ 0.0f, 0.0f, 0.0f }, rotation, { 1.0f, 1.0f, 1.0f }, m_Color);
		GEngine::Renderer2D::DrawQuad({ -1.0f, 0.0f, 0.0f }, rotation, { 2.0f, 1.0f, 1.0f }, m_Color);

		GEngine::Renderer2D::EndScene();*/

		

		GEngine::Renderer2D::BeginScene(m_CameraController->GetCamera());

		for (int i = -(int)m_QuadCount; i < (int)m_QuadCount; i++)
		{
			for (int j = -(int)m_QuadCount; j < (int)m_QuadCount; j++)
			{
				GEngine::Renderer2D::DrawQuad({ (float)i / 10, (float)j / 10, 0.0f }, { 0.0f, 0.0f, 0.0f}, {0.1f, 0.1f, 0.1f}, {(float)i / 10 , (float)j / 10 , 1.0f, 1.0f});
			}
		}

		GEngine::Renderer2D::EndScene();

		GEngine::Renderer2D::BeginScene(m_CameraController->GetCamera());

		for (uint32_t i = 0; i < m_MapWidth; i++)
		{
			for (uint32_t j = 0; j < m_MapHeight; j++)
			{
				char tileType = s_MapTiles[j * s_MapWidth + i];
				GEngine::Ref<GEngine::SubTexture2D> texture;
				if (m_TextureMap.find(tileType) != m_TextureMap.end())
				{
					texture = m_TextureMap[tileType];
				}
				else {

				}

				GEngine::Renderer2D::DrawQuad({ i - m_MapWidth / 2, j - m_MapHeight / 2, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, m_Color, texture);
			}
		}

		GEngine::Renderer2D::EndScene();

		GEngine::Renderer::BeginScene(m_CameraController->GetCamera());

		GEngine::Renderer::EndScene();

		
	}


	if (GEngine::Input::IsMouseButtonPressed(GEngine::MouseCode::Button0))
	{
		auto [x, y] = GEngine::Input::GetMousePosition();
		auto width = GEngine::Application::Get().GetWindow().GetWidth();
		auto height = GEngine::Application::Get().GetWindow().GetHeight();

		auto bounds = m_CameraController->GetBounds();
		auto pos = m_CameraController->GetCamera().GetPosition();
		x = (x / width) * bounds.GetWidth() - bounds.GetWidth() * 0.5f;
		y = bounds.GetHeight() * 0.5f - (y / height) * bounds.GetHeight();
		m_Particle.Position = { x + pos.value.x, y + pos.value.y };
		for (int i = 0; i < 5; i++)
			m_ParticleSystem.Emit(m_Particle);
	}

	m_ParticleSystem.OnUpdate();
	m_ParticleSystem.OnRender(m_CameraController->GetCamera());


}

void Sandbox2D::OnGuiRender()
{
	GE_PROFILE_FUNCTION();
	
	GEngine::ImGuiFunction::DockSpaceBegin();
	bool dockSpace = true;
	GEngine::ImGuiFunction::DockSpace(&dockSpace);

	GEngine::ImGuiFunction::Begin("Settings");
	GEngine::ImGuiFunction::Text("Frames : %llf", 1 / GEngine::Time::GetDeltaTime());
	GEngine::ImGuiFunction::ColorEdit4("m_Color", m_Color.ValuePtr());
	GEngine::ImGuiFunction::Text("Stats:");
	GEngine::ImGuiFunction::Text("Quad Count: %d", GEngine::Renderer2D::GetStats().QuadCount);
	GEngine::ImGuiFunction::Text("Draw Calls: %d", GEngine::Renderer2D::GetStats().DrawCalls);
	GEngine::ImGuiFunction::Text("Vertex Count: %d", GEngine::Renderer2D::GetStats().GetTotalVertexCount());
	GEngine::ImGuiFunction::Text("Index Count: %d", GEngine::Renderer2D::GetStats().GetTotalIndexCount());
	GEngine::ImGuiFunction::SliderFloat("Quads", &m_QuadCount, 1.0f, 100.0f);
	GEngine::ImGuiFunction::End();
	GEngine::ImGuiFunction::DockSpaceEnd();

	
}

void Sandbox2D::OnEvent(GEngine::Event& e)
{
	m_CameraController->OnEvent(e);
}
