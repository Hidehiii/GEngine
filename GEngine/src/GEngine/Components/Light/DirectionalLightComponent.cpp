#include "GEpch.h"
#include "GEngine/Components/Light/DirectionalLightComponent.h"
#include "GEngine/Graphics/Material.h"

#include <cmath>
#include <stdexcept>

namespace GEngine
{
    Vector3 DirectionalLightComponent::GetDirection() const
    {
        const auto direction = m_GameObject.GetTransform().Forward();
        if (!std::isfinite(direction.x) || !std::isfinite(direction.y) || !std::isfinite(direction.z))
            throw std::logic_error("DirectionalLightComponent transform produced a non-finite direction.");
        return direction;
    }

    void DirectionalLightComponent::ApplyToMaterial(const Ref<Material>& material) const
    {
        if (!material) return;
        const auto shader = material->GetShader();
        if (!shader ||
            shader->GetPropertyType("GE_DIRECTIONAL_LIGHT_DIRECTION") != SHADER_PROPERTY_TYPE_FLOAT_3 ||
            shader->GetPropertyType("GE_DIRECTIONAL_LIGHT_COLOR") != SHADER_PROPERTY_TYPE_FLOAT_4 ||
            shader->GetPropertyType("GE_DIRECTIONAL_LIGHT_INTENSITY") != SHADER_PROPERTY_TYPE_FLOAT)
            return;

        material->SetConstant<Vector3>("GE_DIRECTIONAL_LIGHT_DIRECTION", GetDirection());
        material->SetConstant<Vector4>("GE_DIRECTIONAL_LIGHT_COLOR", Color);
        material->SetConstant<float>("GE_DIRECTIONAL_LIGHT_INTENSITY", Intensity);
    }

    void DirectionalLightComponent::ApplyDefaultToMaterial(const Ref<Material>& material)
    {
        if (!material) return;
        const auto shader = material->GetShader();
        if (!shader ||
            shader->GetPropertyType("GE_DIRECTIONAL_LIGHT_DIRECTION") != SHADER_PROPERTY_TYPE_FLOAT_3 ||
            shader->GetPropertyType("GE_DIRECTIONAL_LIGHT_COLOR") != SHADER_PROPERTY_TYPE_FLOAT_4 ||
            shader->GetPropertyType("GE_DIRECTIONAL_LIGHT_INTENSITY") != SHADER_PROPERTY_TYPE_FLOAT)
            return;

        material->SetConstant<Vector3>("GE_DIRECTIONAL_LIGHT_DIRECTION", {0.0f, 0.0f, -1.0f});
        material->SetConstant<Vector4>("GE_DIRECTIONAL_LIGHT_COLOR", {1.0f, 1.0f, 1.0f, 1.0f});
        material->SetConstant<float>("GE_DIRECTIONAL_LIGHT_INTENSITY", 1.0f);
    }
}
