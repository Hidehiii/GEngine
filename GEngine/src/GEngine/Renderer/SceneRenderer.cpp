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
        for (auto object : scene.GetGameObjects<>())
        {
            if (!object.IsActive()) continue;
            if (auto* image = object.TryGetComponent<ImageComponent>()) image->OnRender(command);
            if (auto* mesh = object.TryGetComponent<StaticMeshComponent>()) mesh->OnRender(command);
        }
    }
}
