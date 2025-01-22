#include "Sandbox2D.h"

using namespace GEngine;

static std::filesystem::path s_ModelPath = "Resources\\Model";
static std::filesystem::path s_ShaderPath_2D = "Assets\\Shaders\\2D";
static std::filesystem::path s_ShaderPath_3D = "Assets\\Shaders\\3D";


void* id = nullptr;

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

	FrameBufferSpecification fspec;
	fspec.Attachments = { FrameBufferTextureFormat::RGBA8, FrameBufferTextureFormat::RGBA8, FrameBufferTextureFormat::DEPTH };
	fspec.Width = 720;
	fspec.Height = 720;
	fspec.Samples = 4;
	m_OIT_Present = FrameBuffer::Create(fspec);
	fspec.Samples = 8;
	m_SkyBoxFB = FrameBuffer::Create(fspec);
	fspec.Attachments = { FrameBufferTextureFormat::DEPTH };
	m_DepthOnly = FrameBuffer::Create(fspec);

	m_EditorCamera = Editor::EditorCamera(30.0f, 1.0f, 0.01f, 10000.0f);

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

	m_Scene = CreateRef<Scene>();
	m_SkyBox = m_Scene->CreateGameObject("SkyBox");
	m_SkyBox.AddComponent<MeshFilter>();
	m_ModelImporter.LoadMesh("Resources/Model/2Cube.fbx");
	m_SkyBox.GetComponent<MeshFilter>().SetMesh(m_ModelImporter.GetMesh(0));
	m_SkyBox.AddComponent<MeshRenderer>();
	Ref<Material> mat = Material::Create(Shader::Create("Assets/Shaders/CubeMap.glsl"));
	mat->SetCullMode(CullMode::Front);
	m_SkyBox.GetComponent<MeshRenderer>().SetMaterial(mat);
	m_SkyBox.GetComponent<Transform>().m_Scale *= 100;

	

	m_PresentVertex.resize(4);
	m_OIT = Pipeline::Create(
		Material::Create(Shader::Create("Assets/Shaders/OIT.glsl")),
		VertexBuffer::Create(sizeof(PresentVertex) * m_PresentVertex.size())
	);
	m_OIT->GetVertexBuffer()->SetLayout({
		{ShaderDataType::float4,	"PositionOS"},
		{ShaderDataType::float2,	"UV"}
		});

	m_CopyColorDepth = Pipeline::Create(
		Material::Create(Shader::Create("Assets/Shaders/CopyColorDepth.glsl")),
		VertexBuffer::Create(sizeof(PresentVertex) * m_PresentVertex.size())
	);
	m_CopyColorDepth->GetVertexBuffer()->SetLayout({
		{ShaderDataType::float4,	"PositionOS"},
		{ShaderDataType::float2,	"UV"}
		});

	m_PresentPipeline = Pipeline::Create(
		Material::Create(Shader::Create("Assets/Shaders/Present.glsl")),
		VertexBuffer::Create(sizeof(PresentVertex) * m_PresentVertex.size())
	);
	m_PresentPipeline->GetVertexBuffer()->SetLayout({
		{ShaderDataType::float4,	"PositionOS"},
		{ShaderDataType::float2,	"UV"}
		});
	uint32_t* presentIndices = new uint32_t[6];
	presentIndices[0] = 0;
	presentIndices[1] = 1;
	presentIndices[2] = 2;

	presentIndices[3] = 2;
	presentIndices[4] = 3;
	presentIndices[5] = 0;
	m_PresentPipeline->GetVertexBuffer()->SetIndexBuffer(IndexBuffer::Create(presentIndices, 6));
	m_OIT->GetVertexBuffer()->SetIndexBuffer(IndexBuffer::Create(presentIndices, 6));
	m_CopyColorDepth->GetVertexBuffer()->SetIndexBuffer(IndexBuffer::Create(presentIndices, 6));
	delete[] presentIndices;

	m_PresentVertex[0] = { {-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 0.0f} };
	m_PresentVertex[1] = { { 1.0f, -1.0f, 0.0f, 1.0f}, {1.0f, 0.0f} };
	m_PresentVertex[2] = { { 1.0f,  1.0f, 0.0f, 1.0f}, {1.0f, 1.0f} };
	m_PresentVertex[3] = { {-1.0f,  1.0f, 0.0f, 1.0f}, {0.0f, 1.0f} };

	int count = 12;
	m_vertex.resize(count);
	m_OITPrepare = Pipeline::Create(
		Material::Create(Shader::Create("Assets/Shaders/OITPrepare.glsl")),
		VertexBuffer::Create(sizeof(TestVertex) * m_vertex.size())
	);
	m_OITPrepare->GetVertexBuffer()->SetLayout({
		{ShaderDataType::float4,	"PositionOS"},
		{ShaderDataType::float4,	"Color"}
		});
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
	m_OITPrepare->GetVertexBuffer()->SetIndexBuffer(IndexBuffer::Create(quadIndices, 6 * count / 4));
	delete[] quadIndices;

	
	m_vertex[0].Pos = { -0.5f, -0.5f, 0.0f, 1.0f };
	m_vertex[1].Pos = { 0.5f, -0.5f, 0.0f, 1.0f };
	m_vertex[2].Pos = { 0.5f,  0.5f, 0.0f, 1.0f };
	m_vertex[3].Pos = { -0.5f,  0.5f, 0.0f, 1.0f };
	m_vertex[4].Pos = { -1.0f, -0.5f, -1.0f, 1.0f };
	m_vertex[5].Pos = { 0.0f, -0.5f, -1.0f, 1.0f };
	m_vertex[6].Pos = { 0.0f,  0.5f, -1.0f, 1.0f };
	m_vertex[7].Pos = { -1.0f,  0.5f, -1.0f, 1.0f };
	m_vertex[8].Pos = { -0.5f, -0.5f, 1.0f, 1.0f };
	m_vertex[9].Pos = { 0.5f, -0.5f, 1.0f, 1.0f };
	m_vertex[10].Pos = { 0.5f,  0.5f, -1.5f, 1.0f };
	m_vertex[11].Pos = { -0.5f,  0.5f, -1.5f, 1.0f };

	m_vertex[0].Color = { 1.0f, 0.0f, 0.0f, 0.25f };
	m_vertex[1].Color = { 1.0f, 0.0f, 0.0f, 0.25f };
	m_vertex[2].Color = { 1.0f,  0.0f, 0.0f, 0.25f };
	m_vertex[3].Color = { 1.0f, 0.0f, 0.0f, 0.25f };
	m_vertex[4].Color = { 0.0f, 1.0f, 0.0f, 0.25f };
	m_vertex[5].Color = { 0.0f, 1.0f, 0.0f, 0.25f };
	m_vertex[6].Color = { 0.0f,  1.0f, 0.0f, 0.25f };
	m_vertex[7].Color = { 0.0f,  1.0f, 0.0f, 0.25f };
	m_vertex[8].Color = { 0.0f, 0.0f, 1.0f, 0.25f };
	m_vertex[9].Color = { 0.0f, 0.0f, 1.0f, 0.25f };
	m_vertex[10].Color = { 0.0f,  0.0f, 1.0f, 0.25f };
	m_vertex[11].Color = { 0.0f,  0.0f, 1.0f, 0.25f };


	m_Texture = Texture2D::Create("Assets/Textures/02.png");

	m_OITPrepare->GetVertexBuffer()->SetData(m_vertex.data(), sizeof(TestVertex)* m_vertex.size());
	m_CopyColorDepth->GetVertexBuffer()->SetData(m_PresentVertex.data(), sizeof(PresentVertex) * m_PresentVertex.size());
	m_PresentPipeline->GetVertexBuffer()->SetData(m_PresentVertex.data(), sizeof(PresentVertex)* m_PresentVertex.size());
	m_OIT->GetVertexBuffer()->SetData(m_PresentVertex.data(), sizeof(PresentVertex)* m_PresentVertex.size());


	
	m_StorageImage = StorageImage2D::Create(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight(), ComputeImage2DFormat::R32UI);
	m_ComputeImage2D = StorageImage2D::Create(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight(), ComputeImage2DFormat::RGBA16F);
	m_StorageBuffer = StorageBuffer::Create(Application::Get().GetWindow().GetWidth()* Application::Get().GetWindow().GetHeight() * 4 * sizeof(Node));
	m_SBO = StorageBuffer::Create(sizeof(SBOData));


	m_OITPrepare->GetMaterial()->SetStorageBuffer("GeometrySBO", m_SBO);

	m_OITPrepare->GetMaterial()->SetStorageImage2D("headIndexImage", m_StorageImage);
	m_OIT->GetMaterial()->SetStorageImage2D("headIndexImage", m_StorageImage);


	m_OITPrepare->GetMaterial()->SetStorageBuffer("LinkedListSBO", m_StorageBuffer);
	m_OIT->GetMaterial()->SetStorageBuffer("LinkedListSBO", m_StorageBuffer);

	m_CubeMap = CubeMap::Create("Assets/Textures/right.png",
		"Assets/Textures/left.png",
		"Assets/Textures/top.png",
		"Assets/Textures/bottom.png",
		"Assets/Textures/back.png",
		"Assets/Textures/front.png");
	
	mat->SetCubeMap("TexCube", m_CubeMap);


	m_ComputeTest = ComputePipeline::Create(Material::Create(Shader::Create("Assets/Shaders/ComputeTest.glsl")));
	m_ComputeTest->GetMaterial()->SetStorageImage2D("computeTestImage", m_ComputeImage2D);
	m_ComputeTest->GetMaterial()->SetStorageBuffer("GeometrySBO", m_SBO);
}

void Sandbox2D::OnDetach()
{
}

void Sandbox2D::OnPresent()
{
	// 直接呈现
	Renderer::BeginScene(m_EditorCamera);
	
	m_PresentPipeline->GetMaterial()->SetTexture2D("GE_PRESENT_FRAME_BUFFER", m_OIT_Present->GetColorAttachment(0));
	m_PresentPipeline->GetMaterial()->SetTexture2D("GE_PRESENT_IMGUI", Application::Get().GetImGuiLayer()->GetImGuiImage());
	m_PresentPipeline->Render();

	Renderer::EndScene();
}

void Sandbox2D::OnRender()
{
	
	ComputeCommand::BeginComputeCommand();
	m_ComputeTest->Compute(1, 1, 1);
	ComputeCommand::EndComputeCommand();

	RenderCommand::BeginDrawCommand();
	m_SkyBoxFB->Begin();
	Renderer::BeginScene(m_EditorCamera);
	m_Scene->OnRender();
	Renderer::EndScene();
	m_SkyBoxFB->End();
	RenderCommand::EndDrawCommand();
	

	RenderCommand::BeginDrawCommand();
	m_DepthOnly->Begin();
	Renderer::BeginScene(m_EditorCamera);
	m_OITPrepare->Render();
	Renderer::EndScene();
	m_DepthOnly->End();
	RenderCommand::EndDrawCommand();

	m_CopyColorDepth->GetMaterial()->SetTexture2D("GE_PREVIOUS_COLOR", m_SkyBoxFB->GetColorAttachment(0));
	m_CopyColorDepth->GetMaterial()->SetTexture2D("GE_PREVIOUS_DEPTH", m_SkyBoxFB->GetDepthStencilAttachment());

	RenderCommand::BeginDrawCommand();
	m_OIT_Present->Begin();
	Renderer::BeginScene(m_EditorCamera);
	m_CopyColorDepth->Render();
	m_OIT->Render();
	Renderer::EndScene();
	m_OIT_Present->End();
	RenderCommand::EndDrawCommand();
}

void Sandbox2D::OnUpdate()
{
	//m_EditorCamera.OnUpdate();

	//GEngine::Renderer2D::ResetStats();
	

	//{
	//	// temporary
	//	GEngine::Renderer::BeginScene(m_Camera);

	//	m_RayTracing.OnUpdate(m_EditorCamera);

	//	GEngine::Renderer::EndScene();
	//	
	//}

	m_EditorCamera.OnUpdate(); 

	if (m_OIT_Present->GetHeight() != Application::Get().GetWindow().GetHeight() ||
		m_OIT_Present->GetWidth() != Application::Get().GetWindow().GetWidth())
	{
		m_OIT_Present = FrameBuffer::Recreate(m_OIT_Present, Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
	}
	if (m_DepthOnly->GetHeight() != Application::Get().GetWindow().GetHeight() ||
		m_DepthOnly->GetWidth() != Application::Get().GetWindow().GetWidth())
	{
		m_DepthOnly = FrameBuffer::Recreate(m_DepthOnly, Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
	}
	if (m_SkyBoxFB->GetHeight() != Application::Get().GetWindow().GetHeight() ||
		m_SkyBoxFB->GetWidth() != Application::Get().GetWindow().GetWidth())
	{
		m_SkyBoxFB = FrameBuffer::Recreate(m_SkyBoxFB, Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
	}
}

void Sandbox2D::OnImGuiRender()
{
	
	

	ImGui::Begin("Attachment");
	ImGui::Text("sdadsdasdsdadasdasdad");
	ImGui::End();

	ImGui::Begin("Profile");
	ImGui::Text("Frames : %llf", 1 / GEngine::Time::GetDeltaTime());
	ImGui::Image(GUIUtils::GetTextureID(m_DepthOnly->GetDepthStencilAttachment()), {100, 100});
	//ImGui::Image(GUIUtils::GetTextureID(m_FrameBuffer_0->GetColorAttachment(0)), {100, 100});
	ImGui::End();
}

void Sandbox2D::OnEvent(GEngine::Event& e)
{
	m_EditorCamera.OnEvent(e);
	EventDispatcher dispatcher(e);
	dispatcher.Dispatch<KeyPressedEvent>(GE_BIND_CLASS_FUNCTION_LAMBDA(Sandbox2D::OnKeyPressed));
}

bool Sandbox2D::OnKeyPressed(GEngine::KeyPressedEvent& e)
{
	return false;
}
