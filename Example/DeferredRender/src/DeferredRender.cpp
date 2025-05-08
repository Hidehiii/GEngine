#include "DeferredRender.h"

namespace GEngine
{
    DeferredRender::DeferredRender()
    {
    }
    void DeferredRender::OnAttach()
    {
        ImGui::SetCurrentContext(GEngine::Application::Get().GetImGuiLayer()->GetContext());

        m_EditorCamera = Editor::EditorCamera(30.0f, 1.0f, 0.01f, 10000.0f);

		m_AlbedoTexture = Texture2D::Create("Resources/Textures/Albedo.png");
		m_RoughnessTexture = Texture2D::Create("Resources/Textures/Roughness.png");
		m_MetallicTexture = Texture2D::Create("Resources/Textures/Metallic.png");

        m_ModelImporter.LoadMesh("Resources/Model/Cube.fbx");
        m_ModelImporter.LoadMesh("Resources/Model/Cylinder.fbx");
        m_ModelImporter.LoadMesh("Resources/Model/Sphere.fbx");
        m_ModelImporter.LoadMesh("Resources/Model/Monkey.fbx");
        

        RenderPassSpecification spec;

        spec.ColorRTs = {
            FrameBufferTextureFormat::RGBA8, // Albedo(x y z), Roughness(w)
            FrameBufferTextureFormat::RGBA8, // Normal(x y z), Metallic(w)
			FrameBufferTextureFormat::RGBA8, // SpecularColor(x, y, z), Specular(w)
			FrameBufferTextureFormat::RGBA8, // Positions(x y z)
        };
		spec.DepthStencil = FrameBufferTextureFormat::DEPTH24STENCIL8;
		spec.Samples = 1;

		m_BuildGBufferPass = RenderPass::Create(spec);

        spec.ColorRTs = {
            FrameBufferTextureFormat::RGBA8,
        };

		m_LightingPass = RenderPass::Create(spec);

		m_GBuffer = FrameBuffer::Create(m_BuildGBufferPass, Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
		m_LightingBuffer = FrameBuffer::Create(m_LightingPass, Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());

        PresentVertex vertices[4];

        vertices[0] = { {-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 0.0f} };
        vertices[1] = { { 1.0f, -1.0f, 0.0f, 1.0f}, {1.0f, 0.0f} };
        vertices[2] = { { 1.0f,  1.0f, 0.0f, 1.0f}, {1.0f, 1.0f} };
        vertices[3] = { {-1.0f,  1.0f, 0.0f, 1.0f}, {0.0f, 1.0f} };

		uint32_t indices[6] = {
			0, 1, 2,
			2, 3, 0
		};


        m_CubePipeline = GraphicsPipeline::Create(
            Material::Create("Assets/Shaders/BuildGBuffer.glvk"),
            VertexBuffer::Create(sizeof(Vertex) * m_ModelImporter.GetMesh(0).m_Vertices.size())
        );
        m_CubePipeline->GetVertexBuffer()->SetLayout({
            {ShaderDataType::float4,	"in_Position"},
            {ShaderDataType::float4,	"in_Color"},
            {ShaderDataType::float4,	"in_Normal"},
            {ShaderDataType::float4,	"in_Tangent"},
            {ShaderDataType::float2,	"in_UV0"},
            {ShaderDataType::float2,	"in_UV1"},
            {ShaderDataType::float2,	"in_UV2"},
            {ShaderDataType::float2,	"in_UV3"}
            });
		m_CubePipeline->GetVertexBuffer()->SetIndexBuffer(IndexBuffer::Create(m_ModelImporter.GetMesh(0).m_Indices.data(), m_ModelImporter.GetMesh(0).m_Indices.size()));
		m_CubePipeline->GetVertexBuffer()->SetData(m_ModelImporter.GetMesh(0).m_Vertices.data(), m_ModelImporter.GetMesh(0).m_Vertices.size() * sizeof(Vertex));
		m_CubePipeline->GetMaterial()->SetTexture2D("_Color", m_AlbedoTexture);
		m_CubePipeline->GetMaterial()->SetTexture2D("_Roughness", m_RoughnessTexture);
		m_CubePipeline->GetMaterial()->SetTexture2D("_Metallic", m_MetallicTexture);
		Transform cubeTransform;
		cubeTransform.SetPosition({ 2.0f, 0.0f, 0.0f });
		m_CubePipeline->GetMaterial()->SetMatrix4x4("GE_MATRIX_M", cubeTransform.GetModelMatrix());

		m_CylinderPipeline = GraphicsPipeline::Create(
			Material::Create("Assets/Shaders/BuildGBuffer.glvk"),
			VertexBuffer::Create(sizeof(Vertex) * m_ModelImporter.GetMesh(1).m_Vertices.size())
		);
        m_CylinderPipeline->GetVertexBuffer()->SetLayout({
            {ShaderDataType::float4,	"in_Position"},
            {ShaderDataType::float4,	"in_Color"},
            {ShaderDataType::float4,	"in_Normal"},
            {ShaderDataType::float4,	"in_Tangent"},
            {ShaderDataType::float2,	"in_UV0"},
            {ShaderDataType::float2,	"in_UV1"},
            {ShaderDataType::float2,	"in_UV2"},
            {ShaderDataType::float2,	"in_UV3"}
            });
		m_CylinderPipeline->GetVertexBuffer()->SetIndexBuffer(IndexBuffer::Create(m_ModelImporter.GetMesh(1).m_Indices.data(), m_ModelImporter.GetMesh(1).m_Indices.size()));
		m_CylinderPipeline->GetVertexBuffer()->SetData(m_ModelImporter.GetMesh(1).m_Vertices.data(), m_ModelImporter.GetMesh(1).m_Vertices.size() * sizeof(Vertex));
		m_CylinderPipeline->GetMaterial()->SetTexture2D("_Color", m_AlbedoTexture);
		m_CylinderPipeline->GetMaterial()->SetTexture2D("_Roughness", m_RoughnessTexture);
		m_CylinderPipeline->GetMaterial()->SetTexture2D("_Metallic", m_MetallicTexture);
		Transform cylinderTransform;
		cylinderTransform.SetPosition({ -2.0f, 0.0f, 0.0f });
		m_CylinderPipeline->GetMaterial()->SetMatrix4x4("GE_MATRIX_M", cylinderTransform.GetModelMatrix());

		m_SpherePipeline = GraphicsPipeline::Create(
			Material::Create("Assets/Shaders/BuildGBuffer.glvk"),
			VertexBuffer::Create(sizeof(Vertex) * m_ModelImporter.GetMesh(2).m_Vertices.size())
		);
		m_SpherePipeline->GetVertexBuffer()->SetLayout({
			{ShaderDataType::float4,	"in_Position"},
			{ShaderDataType::float4,	"in_Color"},
			{ShaderDataType::float4,	"in_Normal"},
			{ShaderDataType::float4,	"in_Tangent"},
			{ShaderDataType::float2,	"in_UV0"},
			{ShaderDataType::float2,	"in_UV1"},
			{ShaderDataType::float2,	"in_UV2"},
			{ShaderDataType::float2,	"in_UV3"}
			});
		m_SpherePipeline->GetVertexBuffer()->SetIndexBuffer(IndexBuffer::Create(m_ModelImporter.GetMesh(2).m_Indices.data(), m_ModelImporter.GetMesh(2).m_Indices.size()));
		m_SpherePipeline->GetVertexBuffer()->SetData(m_ModelImporter.GetMesh(2).m_Vertices.data(), m_ModelImporter.GetMesh(2).m_Vertices.size() * sizeof(Vertex));
		m_SpherePipeline->GetMaterial()->SetTexture2D("_Color", m_AlbedoTexture);
		m_SpherePipeline->GetMaterial()->SetTexture2D("_Roughness", m_RoughnessTexture);
		m_SpherePipeline->GetMaterial()->SetTexture2D("_Metallic", m_MetallicTexture);
		Transform sphereTransform;
		sphereTransform.SetPosition({ 0.0f, 2.0f, 0.0f });
		m_SpherePipeline->GetMaterial()->SetMatrix4x4("GE_MATRIX_M", sphereTransform.GetModelMatrix());

        m_MonkeyPipeline = GraphicsPipeline::Create(
			Material::Create("Assets/Shaders/BuildGBuffer.glvk"),
			VertexBuffer::Create(sizeof(Vertex) * m_ModelImporter.GetMesh(3).m_Vertices.size())
        );
		m_MonkeyPipeline->GetVertexBuffer()->SetLayout({
			{ShaderDataType::float4,	"in_Position"},
			{ShaderDataType::float4,	"in_Color"},
			{ShaderDataType::float4,	"in_Normal"},
			{ShaderDataType::float4,	"in_Tangent"},
			{ShaderDataType::float2,	"in_UV0"},
			{ShaderDataType::float2,	"in_UV1"},
			{ShaderDataType::float2,	"in_UV2"},
			{ShaderDataType::float2,	"in_UV3"}
			});
		m_MonkeyPipeline->GetVertexBuffer()->SetIndexBuffer(IndexBuffer::Create(m_ModelImporter.GetMesh(3).m_Indices.data(), m_ModelImporter.GetMesh(3).m_Indices.size()));
		m_MonkeyPipeline->GetVertexBuffer()->SetData(m_ModelImporter.GetMesh(3).m_Vertices.data(), m_ModelImporter.GetMesh(3).m_Vertices.size() * sizeof(Vertex));
		m_MonkeyPipeline->GetMaterial()->SetTexture2D("_Color", m_AlbedoTexture);
		m_MonkeyPipeline->GetMaterial()->SetTexture2D("_Roughness", m_RoughnessTexture);
		m_MonkeyPipeline->GetMaterial()->SetTexture2D("_Metallic", m_MetallicTexture);
		Transform monkeyTransform;
		monkeyTransform.SetPosition({ 0.0f, -2.0f, 0.0f });
		m_MonkeyPipeline->GetMaterial()->SetMatrix4x4("GE_MATRIX_M", monkeyTransform.GetModelMatrix());

		m_LightingPipeline = GraphicsPipeline::Create(
			Material::Create("Assets/Shaders/Lighting.glvk"),
			VertexBuffer::Create(sizeof(PresentVertex) * 4)
		);
		m_LightingPipeline->GetVertexBuffer()->SetLayout({
			{ShaderDataType::float4,	"in_Position"},
			{ShaderDataType::float2,	"in_UV"}
			});
		m_LightingPipeline->GetVertexBuffer()->SetIndexBuffer(IndexBuffer::Create(indices, 6));
		m_LightingPipeline->GetVertexBuffer()->SetData(vertices, sizeof(PresentVertex) * 4);

        m_PresentPipeline = GraphicsPipeline::Create(
            Material::Create("Assets/Shaders/Present.glvk"),
            VertexBuffer::Create(sizeof(PresentVertex) * 4)
        );
		m_PresentPipeline->GetVertexBuffer()->SetLayout({
			{ShaderDataType::float4,	"in_Position"},
			{ShaderDataType::float2,	"in_UV"}
			});
		m_PresentPipeline->GetVertexBuffer()->SetIndexBuffer(IndexBuffer::Create(indices, 6));
		m_PresentPipeline->GetVertexBuffer()->SetData(vertices, sizeof(PresentVertex) * 4);
    }
    void DeferredRender::OnPresent()
    {
		m_PresentPipeline->GetMaterial()->SetTexture2D("_SceneColor", m_LightingBuffer->GetColorRT(0));
		m_PresentPipeline->GetMaterial()->SetTexture2D("_ImguiColor", Application::Get().GetImGuiLayer()->GetImGuiImage());

		Graphics::UpdateCameraUniform(m_EditorCamera);
		GraphicsPresent::Render(m_PresentPipeline, "Present");
    }
    void DeferredRender::OnRender()
    {
		auto buildGBufferCommandBuffer = Graphics::GetGraphicsCommandBuffer();
		auto lightingCommandBuffer = Graphics::GetGraphicsCommandBuffer();

		Graphics::SetCommandsBarrier(buildGBufferCommandBuffer, lightingCommandBuffer);
		Graphics::SetCommandsBarrier(lightingCommandBuffer, GraphicsPresent::GetCommandBuffer());
		Graphics::SetCommandsBarrier(Application::Get().GetImGuiLayer()->GetCommandBuffer(), GraphicsPresent::GetCommandBuffer());

		buildGBufferCommandBuffer->Begin(m_GBuffer);
		Graphics::UpdateCameraUniform(m_EditorCamera);
		buildGBufferCommandBuffer->Render(m_CubePipeline, "BuildGBuffer");
		buildGBufferCommandBuffer->Render(m_CylinderPipeline, "BuildGBuffer");
		buildGBufferCommandBuffer->Render(m_SpherePipeline, "BuildGBuffer");
		buildGBufferCommandBuffer->Render(m_MonkeyPipeline, "BuildGBuffer");
		buildGBufferCommandBuffer->End();

		m_LightingPipeline->GetMaterial()->SetTexture2D("_GBuffer_0", m_GBuffer->GetColorRT(0));
		m_LightingPipeline->GetMaterial()->SetTexture2D("_GBuffer_1", m_GBuffer->GetColorRT(1));
		m_LightingPipeline->GetMaterial()->SetTexture2D("_GBuffer_2", m_GBuffer->GetColorRT(2));
		m_LightingPipeline->GetMaterial()->SetTexture2D("_GBuffer_3", m_GBuffer->GetColorRT(3));


		lightingCommandBuffer->Begin(m_LightingBuffer);
		Graphics::UpdateCameraUniform(m_EditorCamera);
		Graphics::UpdateMainLightUniform(m_LightPosition, m_LightDirection,  m_LightColor);
		lightingCommandBuffer->Render(m_LightingPipeline, "Lighting");
		lightingCommandBuffer->End();
    }
    void DeferredRender::OnUpdate()
    {
		m_EditorCamera.OnUpdate();
    }
    void DeferredRender::OnImGuiRender()
    {
		ImGui::Begin("Deferred Render");
		ImGui::Text("Deferred Render Example");

		Vector4 cubeSpecularColor = m_CubePipeline->GetMaterial()->GetVector("SpecularColor");
		ImGui::ColorEdit4("SpecularColor", Math::ValuePtr(cubeSpecularColor));
		m_CubePipeline->GetMaterial()->SetVector("SpecularColor", cubeSpecularColor);


		m_CylinderPipeline->GetMaterial()->SetVector("SpecularColor", cubeSpecularColor);
		m_SpherePipeline->GetMaterial()->SetVector("SpecularColor", cubeSpecularColor);
		m_MonkeyPipeline->GetMaterial()->SetVector("SpecularColor", cubeSpecularColor);

		ImGui::DragFloat4("LightPosition", Math::ValuePtr(m_LightPosition), 0.1f);
		ImGui::DragFloat4("LightDirection", Math::ValuePtr(m_LightDirection), 0.1f);
		ImGui::DragFloat4("LightColor", Math::ValuePtr(m_LightColor), 0.1f);


		ImGui::Image( GUIUtils::GetTextureID( m_GBuffer->GetColorRT(0)), ImVec2(200, 200),
			ImVec2(GUIUtils::GetUV0().x, GUIUtils::GetUV0().y), ImVec2(GUIUtils::GetUV1().x, GUIUtils::GetUV1().y));
		ImGui::Image(GUIUtils::GetTextureID(m_GBuffer->GetColorRT(1)), ImVec2(200, 200),
			ImVec2(GUIUtils::GetUV0().x, GUIUtils::GetUV0().y), ImVec2(GUIUtils::GetUV1().x, GUIUtils::GetUV1().y));
		ImGui::Image(GUIUtils::GetTextureID(m_GBuffer->GetColorRT(2)), ImVec2(200, 200),
			ImVec2(GUIUtils::GetUV0().x, GUIUtils::GetUV0().y), ImVec2(GUIUtils::GetUV1().x, GUIUtils::GetUV1().y));
		ImGui::Image(GUIUtils::GetTextureID(m_GBuffer->GetColorRT(3)), ImVec2(200, 200),
			ImVec2(GUIUtils::GetUV0().x, GUIUtils::GetUV0().y), ImVec2(GUIUtils::GetUV1().x, GUIUtils::GetUV1().y));
		ImGui::End();
    }
    void DeferredRender::OnEvent(Event& e)
    {
        m_EditorCamera.OnEvent(e);
    }
}