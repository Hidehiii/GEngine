#include "Sandbox2D.h"

using namespace GEngine;

static std::filesystem::path s_ModelPath = "Resources\\Model";
static std::filesystem::path s_ShaderPath_2D = "Assets\\Shaders\\2D";
static std::filesystem::path s_ShaderPath_3D = "Assets\\Shaders\\3D";

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D")
{
	m_Graphics = Graphics::Create();
}

void Sandbox2D::OnAttach()
{
	/*m_EditorCamera = GEngine::Editor::EditorCamera(30.0f, 1.0f, 0.1f, 1000.0f);
	m_Camera = GEngine::Editor::EditorCamera(30.0f, 1.0f, 0.1f, 1000.0f);*/
	//m_RayTracing.Init();
	ImGui::SetCurrentContext(GEngine::Application::Get().GetImGuiLayer()->GetContext());

	FrameBufferSpecification fspec;
	fspec.Attachments = { FrameBufferTextureFormat::RGBA8, FrameBufferTextureFormat::DEPTH };
	fspec.Width = 720;
	fspec.Height = 720;
	m_FrameBuffer = FrameBuffer::Create(fspec);

	m_EditorCamera = Editor::EditorCamera(10.0f, 1.0f, 0.01f, 10000.0f);

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

	int count = 8;
	m_vertex.resize(count);
	m_Pipeline = Pipeline::Create(
		Material::Create(Shader::Create("Assets/Shaders/2D/ttt.glsl")),
		VertexArray::Create(),
		VertexBuffer::Create(sizeof(TestVertex) * m_vertex.size())
	);
	m_Pipeline->GetVertexBuffer()->SetLayout({
		{ShaderDataType::float4,	"PositionOS"},
		{ShaderDataType::float2,	"UV"},
		{ShaderDataType::int1,		"TexIndex"}
		});
	m_Pipeline->GetVertexArray()->SetVertexBuffer(m_Pipeline->GetVertexBuffer());
	uint32_t* quadIndices = new uint32_t[6 * count / 4];
	uint32_t offset = 0;
	for (uint32_t i = 0; i < 6 * count / 4; i += 6)
	{
		quadIndices[i + 0] = offset + 0;
		quadIndices[i + 1] = offset + 1;
		quadIndices[i + 2] = offset + 2;

		quadIndices[i + 3] = offset + 2;
		quadIndices[i + 4] = offset + 3;
		quadIndices[i + 5] = offset + 0;

		offset += 4;
	}
	m_Pipeline->GetVertexArray()->SetIndexBuffer(IndexBuffer::Create(quadIndices, 6 * count / 4));
	delete[] quadIndices;

	
	
	m_vertex[0].Pos = { -0.5f, -0.5f, 0.0f, 1.0f };
	m_vertex[1].Pos = { 0.5f, -0.5f, 0.0f, 1.0f };
	m_vertex[2].Pos = { 0.5f,  0.5f, 0.0f, 1.0f };
	m_vertex[3].Pos = { -0.5f,  0.5f, 0.0f, 1.0f };
	m_vertex[4].Pos = { -1.0f, -0.5f, -1.0f, 1.0f };
	m_vertex[5].Pos = { 0.0f, -0.5f, -1.0f, 1.0f };
	m_vertex[6].Pos = { 0.0f,  0.5f, -1.0f, 1.0f };
	m_vertex[7].Pos = { -1.0f,  0.5f, -1.0f, 1.0f };

	m_vertex[0].UV = { 0.0f, 0.0f };
	m_vertex[1].UV = { 1.0f, 0.0f };
	m_vertex[2].UV = { 1.0f, 1.0f };
	m_vertex[3].UV = { 0.0f, 1.0f };
	m_vertex[4].UV = { 0.0f, 0.0f };
	m_vertex[5].UV = { 1.0f, 0.0f };
	m_vertex[6].UV = { 1.0f, 1.0f };
	m_vertex[7].UV = { 0.0f, 1.0f };

	m_vertex[0].index = 0;
	m_vertex[1].index = 0;
	m_vertex[2].index = 0;
	m_vertex[3].index = 0;
	m_vertex[4].index = 0;
	m_vertex[5].index = 0;
	m_vertex[6].index = 0;
	m_vertex[7].index = 0;

	m_Texture = Texture2D::Create(1, 1);
	uint32_t whiteTextureData = 0xffffffff;
	m_Texture->SetData(&whiteTextureData, sizeof(whiteTextureData));

	m_Pipeline->GetMaterial()->SetFloat("prop1", 1.0f);
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
	//	GEngine::Renderer::BeginScene(m_Camera);

	//	m_RayTracing.OnUpdate(m_EditorCamera);

	//	GEngine::Renderer::EndScene();
	//	
	//}

	m_EditorCamera.OnUpdate();
#if 0
	RenderCommand::BeginCommand();
	m_FrameBuffer->Begin();
	RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	RenderCommand::Clear();

	
	Renderer::BeginScene(m_EditorCamera);
	m_Texture->Bind(0);
	m_Pipeline->GetVertexBuffer()->SetData(m_vertex.data(), sizeof(TestVertex) * m_vertex.size());
	m_Pipeline->Bind();
	RenderCommand::DrawTriangles(m_Pipeline->GetVertexArray());
	Renderer::EndScene();
	m_FrameBuffer->End();
	RenderCommand::EndCommand();

#else
	RenderCommand::SetClearColor(Vector4( 0.0f, 0.0f, 0.0f, 1.0f ));
	RenderCommand::Clear();
	// 直接呈现
	m_Graphics->Begin();
	RenderCommand::SetClearColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	RenderCommand::Clear();

	Renderer::BeginScene(m_EditorCamera);
	m_Pipeline->GetMaterial()->SetCullMode(MaterialCullMode::None);
	m_Texture->Bind(0);
	m_Pipeline->GetVertexBuffer()->SetData(m_vertex.data(), sizeof(TestVertex) * m_vertex.size());
	m_Pipeline->Bind();
	RenderCommand::DrawTriangles(m_Pipeline->GetVertexArray());
	Renderer::EndScene();
	m_Graphics->End();
#endif
}

void Sandbox2D::OnGuiRender()
{
	GE_PROFILE_FUNCTION();
	/*ImGui::Begin("Profile");
	ImGui::Text("Frames : %llf", 1 / GEngine::Time::GetDeltaTime());
	uint32_t tex = m_FrameBuffer->GetColorAttachment();
	ImGui::Image((void*)tex, ImVec2(128, 72), { 0.0f, 1.0f }, { 1.0f, 0.0f });
	ImGui::End();*/
}

void Sandbox2D::OnEvent(GEngine::Event& e)
{
	m_EditorCamera.OnEvent(e);
	EventDispatcher dispatcher(e);
	dispatcher.Dispatch<KeyPressedEvent>(GE_BIND_EVENT_FN(Sandbox2D::OnKeyPressed));
	GE_INFO(e.ToString());
}

bool Sandbox2D::OnKeyPressed(GEngine::KeyPressedEvent& e)
{
	if (e.GetKeyCode() == KeyCode::A && e.IsRepeat() == false)
	{	
		if(m_Pipeline->GetMaterial()->GetFloat("prop1") == 1.0f)
			m_Pipeline->GetMaterial()->SetFloat("prop1", -1.0f);
		else
			m_Pipeline->GetMaterial()->SetFloat("prop1", 1.0f);
	}
	return false;
}
