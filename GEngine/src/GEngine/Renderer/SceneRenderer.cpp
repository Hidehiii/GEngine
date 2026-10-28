#include "GEpch.h"
#include "GEngine/Renderer/SceneRenderer.h"
#include "GEngine/Object/GameObject.h"
#include "GEngine/Components/Components.h"
#include <stdexcept>

namespace GEngine
{
    void SceneRenderer::Render(Scene& scene, const Ref<CommandBuffer>& command)
    {
        if (!command) throw std::invalid_argument("SceneRenderer requires a command buffer.");
        for (auto object : scene.GetGameObjects<Attribute, Transform>())
        {
            if (!object.GetComponent<Attribute>().m_IsActive) continue;
            if (auto* image = object.TryGetComponent<ImageRenderer>()) image->OnRender(command);
            if (auto* mesh = object.TryGetComponent<MeshRenderer>()) mesh->OnRender(command);
        }
    }
}
