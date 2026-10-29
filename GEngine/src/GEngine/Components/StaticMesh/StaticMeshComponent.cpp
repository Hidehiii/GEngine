#include "GEpch.h"
#include "GEngine/Components/StaticMesh/StaticMeshComponent.h"
#include "GEngine/Tools/ModelImporter.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Graphics/GraphicsPresent.h"
#include "GEngine/Graphics/CommandBuffer.h"
#include <stdexcept>
namespace GEngine
{
    StaticMeshComponent::StaticMeshComponent(const StaticMeshComponent& other)
        : Component(other), m_Mesh(other.m_Mesh), m_Material(other.m_Material), m_Path(other.m_Path), m_Index(other.m_Index) {}
    void StaticMeshComponent::SetMeshAsset(const Ref<const Mesh>& mesh)
    {
        if (!mesh || !*mesh) throw std::invalid_argument("StaticMeshComponent requires a nonempty mesh asset.");
        if (mesh->m_Vertices.size() > UINT32_MAX / sizeof(Vertex) || mesh->m_Indices.size() > UINT32_MAX ||
            std::any_of(mesh->m_Indices.begin(), mesh->m_Indices.end(), [&](uint32_t index) { return index >= mesh->m_Vertices.size(); }))
            throw std::invalid_argument("StaticMeshComponent asset has an invalid size or index range.");
        m_Mesh = mesh;
        m_Path.clear();
        m_Index = 0;
        m_Pipeline.reset();
    }
    void StaticMeshComponent::LoadMeshAsset(const std::string& path, uint32_t index)
    {
        if (std::filesystem::u8path(path).extension() != ".gasset")
            throw std::invalid_argument("StaticMeshComponent asset must be a mesh .gasset.");
        ModelImporter importer;
        if (!importer.LoadMesh(path)) throw std::runtime_error("Cannot load mesh asset: " + path);
        SetMeshAsset(CreateRef<Mesh>(importer.GetMesh(static_cast<int>(index))));
        m_Path = path; m_Index = index;
    }
    void StaticMeshComponent::SetMaterial(const Ref<Material>& material)
    {
        if (!material) throw std::invalid_argument("StaticMeshComponent requires a material.");
        if (!material->GetShader() || material->GetShader()->GetPropertyType("GE_MATRIX_M") != SHADER_PROPERTY_TYPE_MATRIX_4X4)
            throw std::invalid_argument("StaticMeshComponent material requires GE_MATRIX_M.");
        m_Material = material; m_Pipeline.reset();
    }
    void StaticMeshComponent::OnRender() { OnRender(GraphicsPresent::GetCommandBuffer()); }
    void StaticMeshComponent::OnRender(const Ref<CommandBuffer>& command)
    {
        if (!command || !m_Mesh || !m_Material)
            throw std::logic_error("StaticMeshComponent requires a command buffer, mesh asset and material.");
        if (!m_Pipeline)
        {
            auto& device = Graphics::GetRenderDevice();
            auto buffer = device.CreateVertexBuffer(m_Mesh->m_Vertices.data(),
                static_cast<uint32_t>(m_Mesh->m_Vertices.size() * sizeof(Vertex)), 0, VERTEX_TOPOLOGY_TRIANGLE);
            buffer->SetIndexBuffer(device.CreateIndexBuffer(m_Mesh->m_Indices.data(), static_cast<uint32_t>(m_Mesh->m_Indices.size())));
            m_Pipeline = device.CreateGraphicsPipeline(m_Material, buffer);
        }
        m_Material->SetConstant<Matrix4x4>("GE_MATRIX_M", m_GameObject.GetTransform().GetModelMatrix());
        command->Render(m_Pipeline, 0);
    }
}
