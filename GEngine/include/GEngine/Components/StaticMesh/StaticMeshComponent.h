#pragma once
#include "GEngine/Components/Component.h"
#include "GEngine/Asset/Mesh.h"
#include "GEngine/Graphics/GraphicsPipeline.h"
namespace GEngine
{
    class GENGINE_API StaticMeshComponent : public Component
    {
    public:
        StaticMeshComponent() = default;
        StaticMeshComponent(const StaticMeshComponent& other);
        void SetMeshAsset(const Ref<const Mesh>& mesh);
        void LoadMeshAsset(const std::string& path, uint32_t index = 0);
        Ref<const Mesh> GetMeshAsset() const { return m_Mesh; }
        const std::string& GetMeshAssetPath() const { return m_Path; }
        uint32_t GetMeshIndex() const { return m_Index; }
        void SetMaterial(const Ref<Material>& material);
        Ref<Material> GetMaterial() const { return m_Material; }
        void OnRender() override;
        void OnRender(const Ref<CommandBuffer>& command);
    private:
        Ref<const Mesh> m_Mesh;
        Ref<Material> m_Material;
        Ref<GraphicsPipeline> m_Pipeline;
        std::string m_Path;
        uint32_t m_Index = 0;
    };
}
