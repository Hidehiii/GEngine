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

	Ref<RenderPass> OpaquePass, DepthOnlyPass;

	RenderPassSpecification spec;
	spec.ColorAttachments = { FrameBufferTextureFormat::RGBA8, FrameBufferTextureFormat::RGBA8 };
	spec.DepthAttachment = FrameBufferTextureFormat::DEPTH;
	spec.Samples = 4;
	OpaquePass = RenderPass::Create(spec);

	spec.ColorAttachments = {};
	spec.Samples = 1;
	DepthOnlyPass = RenderPass::Create(spec);

	m_OIT_Present = FrameBuffer::Create(OpaquePass, 720, 720);

	m_SkyBoxFB = FrameBuffer::Create(OpaquePass, 720, 720);

	m_DepthOnly = FrameBuffer::Create(DepthOnlyPass, 720, 720);

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
				Shader::Create(path.string());
			}
		}
		for (auto& shaderFile : std::filesystem::directory_iterator(s_ShaderPath_3D))
		{
			const auto& path = shaderFile.path();
			std::string filenameString = path.filename().string();
			if (path.extension() == ".glsl" || path.extension() == ".GLSL")
			{
				GE_TRACE("Shader: {0} loading", path);
				Shader::Create(path.string());
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
	m_OIT = GraphicsPipeline::Create(
		Material::Create(Shader::Create("Assets/Shaders/OIT.glsl")),
		VertexBuffer::Create(sizeof(PresentVertex) * m_PresentVertex.size())
	);
	m_OIT->GetVertexBuffer()->SetLayout({
		{ShaderDataType::float4,	"PositionOS"},
		{ShaderDataType::float2,	"UV"}
		});

	m_CopyColorDepth = GraphicsPipeline::Create(
		Material::Create(Shader::Create("Assets/Shaders/CopyColorDepth.glsl")),
		VertexBuffer::Create(sizeof(PresentVertex) * m_PresentVertex.size())
	);
	m_CopyColorDepth->GetVertexBuffer()->SetLayout({
		{ShaderDataType::float4,	"PositionOS"},
		{ShaderDataType::float2,	"UV"}
		});

	m_PresentPipeline = GraphicsPipeline::Create(
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
	m_OITPrepare = GraphicsPipeline::Create(
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

	m_CubeMap->SetData(m_Texture, m_CubeMap->GetWidth(), m_CubeMap->GetHeight(), CubeMap::CubeMapFace::Front);
	
	mat->SetCubeMap("TexCube", m_CubeMap);


	m_ComputeTest = ComputePipeline::Create(Material::Create(Shader::Create("Assets/Shaders/ComputeTest.glsl")));
	m_ComputeTest->GetMaterial()->SetStorageImage2D("computeTestImage", m_ComputeImage2D);
	m_ComputeTest->GetMaterial()->SetStorageBuffer("GeometrySBO", m_SBO);

	/*auto exts = RenderCommand::GetExtensions();
	for (auto e : exts)
	{
		GE_INFO(e);
	}*/

	/*m_SeparateTextureSampler = GraphicsPipeline::Create(
		Material::Create(Shader::Create("Assets/Shaders/SperateTexture2DAndSampler.glsl")),
		VertexBuffer::Create(sizeof(TestVertex) * m_vertex.size())
	);
	m_SeparateTextureSampler->GetVertexBuffer()->SetLayout({
		{ShaderDataType::float4,	"PositionOS"}
		});
	m_SeparateTextureSampler->GetVertexBuffer()->SetIndexBuffer(IndexBuffer::Create(presentIndices, 6));
	m_SeparateTextureSampler->GetVertexBuffer()->SetData(m_PresentVertex.data(), sizeof(PresentVertex)* m_PresentVertex.size());*/
}

void Sandbox2D::OnPresent()
{
	// 直接呈现
	m_PresentPipeline->GetMaterial()->SetTexture2D("GE_PRESENT_FRAME_BUFFER", m_OIT_Present->GetColorRT(0));
	m_PresentPipeline->GetMaterial()->SetTexture2D("GE_PRESENT_IMGUI", Application::Get().GetImGuiLayer()->GetImGuiImage());

	Graphics::UpdateCameraUniform(m_EditorCamera);
	GraphicsPresent::Render(m_PresentPipeline);
}

void Sandbox2D::OnRender()
{
	auto cmd0 = Graphics::GetComputeCommandBuffer();
	auto cmd1 = Graphics::GetGraphicsCommandBuffer();
	auto cmd2 = Graphics::GetGraphicsCommandBuffer();
	auto cmd3 = Graphics::GetGraphicsCommandBuffer();

	Graphics::SetCommandsBarrier(cmd0, cmd1);
	Graphics::SetCommandsBarrier(cmd0, cmd2);
	Graphics::SetCommandsBarrier(cmd1, cmd3);
	Graphics::SetCommandsBarrier(cmd2, cmd3);
	Graphics::SetCommandsBarrier(cmd3, Application::Get().GetImGuiLayer()->GetCommandBuffer());
	Graphics::SetCommandsBarrier(Application::Get().GetImGuiLayer()->GetCommandBuffer(), GraphicsPresent::GetCommandBuffer());

	cmd0->Begin(m_DepthOnly);
	Graphics::UpdateCameraUniform(m_EditorCamera);
	cmd0->Compute(m_ComputeTest, 1, 1, 1);
	cmd0->End();

	cmd1->Begin(m_SkyBoxFB);
	Graphics::UpdateCameraUniform(m_EditorCamera);
	cmd1->Render(m_Scene);
	cmd1->End();

	cmd2->Begin(m_DepthOnly);
	Graphics::UpdateCameraUniform(m_EditorCamera);
	cmd2->Render(m_OITPrepare);
	cmd2->End();

	m_CopyColorDepth->GetMaterial()->SetTexture2D("GE_PREVIOUS_COLOR", m_SkyBoxFB->GetColorRT(0));
	m_CopyColorDepth->GetMaterial()->SetTexture2D("GE_PREVIOUS_DEPTH", m_SkyBoxFB->GetDepthStencilRT());
	m_OIT->GetMaterial()->SetTexture2D("OpaqueColor", m_SkyBoxFB->GetColorRT(0));

	cmd3->Begin(m_OIT_Present);
	Graphics::UpdateCameraUniform(m_EditorCamera);
	cmd3->Render(m_CopyColorDepth);
	cmd3->Render(m_OIT);
	cmd3->End();
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
		m_OIT_Present = FrameBuffer::Create(m_OIT_Present, Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
	}
	if (m_DepthOnly->GetHeight() != Application::Get().GetWindow().GetHeight() ||
		m_DepthOnly->GetWidth() != Application::Get().GetWindow().GetWidth())
	{
		m_DepthOnly = FrameBuffer::Create(m_DepthOnly, Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
	}
	if (m_SkyBoxFB->GetHeight() != Application::Get().GetWindow().GetHeight() ||
		m_SkyBoxFB->GetWidth() != Application::Get().GetWindow().GetWidth())
	{
		m_SkyBoxFB = FrameBuffer::Create(m_SkyBoxFB, Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
	}

	m_PresentPipeline->GetMaterial()->SetFloat("f", 1.0f);
	m_PresentPipeline->GetMaterial()->SetVector("v", Vector4(1.0f, 2.0f, 3.0f, 0.0f));

	GE_INFO(m_PresentPipeline->GetMaterial()->GetFloat("f"));
	GE_INFO(m_PresentPipeline->GetMaterial()->GetVector("v"));
}

void Sandbox2D::OnImGuiRender()
{


	ImGui::Begin("Attachment");
	ImGui::Text("sdadsdasdsdadasdasdad");
	ImGui::End();

	ImGui::Begin("Profile");
	ImGui::Text("Frames : %llf", 1 / GEngine::Time::GetDeltaTime());
	ImGui::Image(GUIUtils::GetTextureID(m_DepthOnly->GetDepthStencilRT()), {100, 100});
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
