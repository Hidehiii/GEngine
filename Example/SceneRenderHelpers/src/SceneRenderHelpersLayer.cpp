#include "SceneRenderHelpersLayer.h"
#include <GEngine/Renderer/SceneRenderer.h>

#include <GEngine/Tools/GraphicsTool.h>
#include <GEngine/Events/ApplicationEvent.h>
#include <cstdlib>
#include <stdexcept>

namespace GEngine
{
	SceneRenderHelpersLayer::SceneRenderHelpersLayer()
		: Layer("SceneRenderHelpers")
	{
	}

	void SceneRenderHelpersLayer::OnAttach()
	{
		auto expectUnsupported = [](auto&& operation, const char* name)
		{
			bool rejected = false;
			try { operation(); }
			catch (const std::logic_error&) { rejected = true; }
			if (!rejected)
				throw std::runtime_error(std::string(name) + " unexpectedly succeeded.");
		};
		expectUnsupported([] { GraphicsTool::Init(); }, "GraphicsTool::Init");
		expectUnsupported([] { GraphicsTool::RenderPoint({ 0.0f, 0.0f, 0.0f }); }, "GraphicsTool::RenderPoint");
		expectUnsupported([] { GraphicsTool::RenderLine({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }); }, "GraphicsTool::RenderLine");

		m_Scene = CreateRef<Scene>();
		m_Image = m_Scene->CreateGameObject("Color quad");
		m_Image.GetComponent<Transform>().SetPosition({ -0.5f, 0.0f, 0.0f });
		m_Image.GetComponent<Transform>().SetScale({ 0.65f, 0.65f, 1.0f });
		m_Image.AddComponent<ImageRenderer>(Vector4{ 0.95f, 0.22f, 0.18f, 1.0f });
		auto& imageRenderer = m_Image.GetComponent<ImageRenderer>();
		imageRenderer.SetMaterial(Material::Create(Shader::Create("Assets/Shaders/ImageColor.gasset"), "ImageColor"));
		imageRenderer.m_Texture = Texture2D::White();
		expectUnsupported([&imageRenderer] { imageRenderer.OnRender(); }, "Textured ImageRenderer");
		imageRenderer.m_Texture.reset();

		m_Mesh = m_Scene->CreateGameObject("Indexed mesh");
		m_Mesh.GetComponent<Transform>().SetPosition({ 0.5f, 0.0f, 0.0f });
		m_Mesh.AddComponent<MeshFilter>();
		Mesh mesh;
		mesh.m_Vertices.resize(3);
		mesh.m_Vertices[0].m_Position = { -0.42f, -0.48f, 0.0f, 1.0f };
		mesh.m_Vertices[1].m_Position = {  0.42f, -0.48f, 0.0f, 1.0f };
		mesh.m_Vertices[2].m_Position = {  0.0f,   0.48f, 0.0f, 1.0f };
		for (auto& vertex : mesh.m_Vertices)
			vertex.m_Color = { 0.10f, 0.72f, 0.95f, 1.0f };
		mesh.m_Indices = { 0, 1, 2 };
		m_Mesh.GetComponent<MeshFilter>().SetMesh(mesh);
		m_Mesh.AddComponent<MeshRenderer>();
		m_Mesh.GetComponent<MeshRenderer>().SetMaterial(
			Material::Create(Shader::Create("Assets/Shaders/MeshColor.gasset"), "MeshColor"));
		GE_INFO("SceneRenderHelpers initialized: color quad, indexed mesh, unsupported paths rejected.");
	}

	void SceneRenderHelpersLayer::OnUpdate()
	{
		if (m_FrameCount == 30)
		{
			auto& renderer = m_Image.GetComponent<ImageRenderer>();
			renderer.m_Color = { 0.95f, 0.72f, 0.18f, 1.0f };
			renderer.SetMaterial(Material::Create(Shader::Create("Assets/Shaders/ImageColor.gasset"), "ReplacedImageColor"));
			GE_INFO("SceneRenderHelpers replaced image material after 30 frames.");
		}
		if (m_FrameCount == 60)
		{
			Mesh mesh = m_Mesh.GetComponent<MeshFilter>().GetMesh();
			mesh.m_Vertices[2].m_Position.y = 0.65f;
			m_Mesh.GetComponent<MeshFilter>().SetMesh(mesh);
			GE_INFO("SceneRenderHelpers replaced mesh vertices after 60 frames.");
		}
		static const bool syntheticResize = []
		{
			const char* value = std::getenv("GENGINE_SCENE_HELPER_SYNTHETIC_RESIZE");
			return value && value[0] == '1' && value[1] == '\0';
		}();
		if (syntheticResize && Graphics::GetGraphicsAPI() == GRAPHICS_API_DIRECT3DX12 &&
			((m_FrameCount == 40 && !m_FirstSyntheticResizeRequested) ||
			 (m_FrameCount == 80 && !m_SecondSyntheticResizeRequested)))
		{
			if (m_FrameCount == 40)
				m_FirstSyntheticResizeRequested = true;
			else
				m_SecondSyntheticResizeRequested = true;
			const uint32_t width = m_FrameCount == 40 ? 960 : 1280;
			const uint32_t height = m_FrameCount == 40 ? 540 : 720;
			WindowResizeEvent resize(width, height);
			Application::Get().OnEvent(resize);
			GE_INFO("SceneRenderHelpers requested synthetic resize: {}x{}.", width, height);
		}
	}

	void SceneRenderHelpersLayer::OnPresent()
	{
		if (m_FrameCount % 2 == 0)
			SceneRenderer::Render(*m_Scene, GraphicsPresent::GetCommandBuffer());
		else
		{
			m_Image.OnRender();
			m_Mesh.OnRender();
		}
		++m_FrameCount;
		static const uint64_t frameLimit = []
		{
			const char* value = std::getenv("GENGINE_SCENE_HELPER_FRAME_LIMIT");
			return value ? std::strtoull(value, nullptr, 10) : 0ull;
		}();
		if (frameLimit && m_FrameCount >= frameLimit)
		{
			GE_INFO("SceneRenderHelpers completed {} frames; requesting normal shutdown.", m_FrameCount);
			Application::Get().Close();
		}
	}

	void SceneRenderHelpersLayer::OnEndFrame()
	{
		m_Scene->OnEndFrame();
	}
}
