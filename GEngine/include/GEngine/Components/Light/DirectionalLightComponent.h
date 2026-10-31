#pragma once

#include "GEngine/Components/Component.h"
#include "GEngine/Math/Math.h"

namespace GEngine
{
    class Material;

    class GENGINE_API DirectionalLightComponent final : public Component
    {
    public:
        DirectionalLightComponent() = default;
        DirectionalLightComponent(const DirectionalLightComponent& other) = default;

        Vector3 GetDirection() const;
        void ApplyToMaterial(const Ref<Material>& material) const;
        static void ApplyDefaultToMaterial(const Ref<Material>& material);

        Vector4 Color = {1.0f, 1.0f, 1.0f, 1.0f};
        float Intensity = 1.0f;
    };
}
