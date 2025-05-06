#include "DeferredRender.h"

namespace GEngine
{
    DeferredRender::DeferredRender()
    {
    }
    void DeferredRender::OnAttach()
    {
        ImGui::SetCurrentContext(GEngine::Application::Get().GetImGuiLayer()->GetContext());

        m_ModelImporter.LoadMesh("Resources/Model/Cube.fbx");
        m_ModelImporter.LoadMesh("Resources/Model/Cylinder.fbx");
        m_ModelImporter.LoadMesh("Resources/Model/Monkey.fbx");
        m_ModelImporter.LoadMesh("Resources/Model/Sphere.fbx");

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
    }
    void DeferredRender::OnPresent()
    {
    }
    void DeferredRender::OnRender()
    {
    }
    void DeferredRender::OnUpdate()
    {
    }
    void DeferredRender::OnImGuiRender()
    {
    }
    void DeferredRender::OnEvent(Event& e)
    {
    }
}