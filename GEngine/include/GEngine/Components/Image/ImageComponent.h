#pragma once
#include "GEngine/Components/Component.h"
#include "GEngine/Graphics/Texture.h"
#include "GEngine/Graphics/GraphicsPipeline.h"
#include "GEngine/Graphics/Sampler.h"
namespace GEngine
{
    class GENGINE_API ImageComponent : public Component
    {
    public:
        explicit ImageComponent(Vector4 color = {1, 1, 1, 1}) : Color(color) {}
        ImageComponent(const ImageComponent& other);
        void SetImageAsset(const Ref<Texture2D>& image);
        void LoadImageAsset(const std::string& path);
        Ref<Texture2D> GetImageAsset() const { return m_Image; }
        void SetMaterial(const Ref<Material>& material);
        Ref<Material> GetMaterial() const { return m_Material; }
        void OnRender() override;
        void OnRender(const Ref<CommandBuffer>& command);
        Vector4 Color = {1, 1, 1, 1};
        Vector2 Tiling = {1, 1};
    private:
        Ref<Texture2D> m_Image;
        Ref<Material> m_Material;
        Ref<GraphicsPipeline> m_Pipeline;
        Ref<Sampler> m_Sampler;
    };
}
