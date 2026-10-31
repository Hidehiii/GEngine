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
        const DirectionalLightComponent* directionalLight = nullptr;
        for (auto object : scene.GetGameObjects<>())
        {
            if (!object.IsActive()) continue;
            if (auto* light = object.TryGetComponent<DirectionalLightComponent>())
            {
                directionalLight = light;
                break;
            }
        }
        for (auto object : scene.GetGameObjects<>())
        {
            if (!object.IsActive()) continue;
            if (auto* image = object.TryGetComponent<ImageComponent>())
            {
                if (directionalLight) directionalLight->ApplyToMaterial(image->GetMaterial());
                else DirectionalLightComponent::ApplyDefaultToMaterial(image->GetMaterial());
                image->OnRender(command);
            }
            if (auto* mesh = object.TryGetComponent<StaticMeshComponent>())
            {
                if (directionalLight) directionalLight->ApplyToMaterial(mesh->GetMaterial());
                else DirectionalLightComponent::ApplyDefaultToMaterial(mesh->GetMaterial());
                mesh->OnRender(command);
            }
        }
    }
}
