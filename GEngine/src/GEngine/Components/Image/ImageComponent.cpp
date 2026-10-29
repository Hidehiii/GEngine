#include "GEpch.h"
#include "GEngine/Components/Image/ImageComponent.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Graphics/GraphicsPresent.h"
#include "GEngine/Graphics/CommandBuffer.h"
#include <stdexcept>
#include <filesystem>
namespace GEngine
{
    ImageComponent::ImageComponent(const ImageComponent& other)
        : Component(other), Color(other.Color), Tiling(other.Tiling), m_Image(other.m_Image), m_Material(other.m_Material) {}
    void ImageComponent::SetImageAsset(const Ref<Texture2D>& image) { m_Image = image; }
    void ImageComponent::LoadImageAsset(const std::string& path)
    {
        if (std::filesystem::u8path(path).extension() != ".gasset")
            throw std::invalid_argument("ImageComponent asset must be a texture .gasset.");
        SetImageAsset(Graphics::GetRenderDevice().CreateTexture2D(path));
    }
    void ImageComponent::SetMaterial(const Ref<Material>& material)
    {
        if (!material) throw std::invalid_argument("ImageComponent requires a material.");
        const auto shader = material->GetShader();
        if (!shader || shader->GetPropertyType("GE_MATRIX_M") != SHADER_PROPERTY_TYPE_MATRIX_4X4 ||
            shader->GetPropertyType("GE_IMAGE_COLOR") != SHADER_PROPERTY_TYPE_FLOAT_4 ||
            shader->GetPropertyType("GE_IMAGE_TILING") != SHADER_PROPERTY_TYPE_FLOAT_2 ||
            shader->GetPropertyType("GE_IMAGE_TEXTURE") != SHADER_PROPERTY_TYPE_TEXTURE_2D ||
            shader->GetPropertyType("GE_IMAGE_SAMPLER") != SHADER_PROPERTY_TYPE_SAMPLER)
            throw std::invalid_argument("ImageComponent material does not provide the required constants and sampled image bindings.");
        m_Material = material; m_Pipeline.reset();
    }
    void ImageComponent::OnRender() { OnRender(GraphicsPresent::GetCommandBuffer()); }
    void ImageComponent::OnRender(const Ref<CommandBuffer>& command)
    {
        if (!command || !m_Material) throw std::logic_error("ImageComponent requires a command buffer and material.");
        if (!m_Pipeline)
        {
            const float vertices[][5] = {
                {-0.5f, -0.5f, 0, 0, 0}, {0.5f, -0.5f, 0, 1, 0},
                {0.5f, 0.5f, 0, 1, 1}, {-0.5f, 0.5f, 0, 0, 1}};
            const uint32_t indices[] = {0, 1, 2, 2, 3, 0};
            auto& device = Graphics::GetRenderDevice();
            auto buffer = device.CreateVertexBuffer(vertices, sizeof(vertices), 0, VERTEX_TOPOLOGY_TRIANGLE);
            buffer->SetIndexBuffer(device.CreateIndexBuffer(indices, 6));
            m_Pipeline = device.CreateGraphicsPipeline(m_Material, buffer);
        }
        if (!m_Sampler) m_Sampler = Sampler::GetDefaultSampler();
        m_Material->SetConstant<Matrix4x4>("GE_MATRIX_M", m_GameObject.GetTransform().GetModelMatrix());
        m_Material->SetConstant<Vector4>("GE_IMAGE_COLOR", Color);
        m_Material->SetConstant<Vector2>("GE_IMAGE_TILING", Tiling);
        m_Material->SetResource<Texture2D>("GE_IMAGE_TEXTURE", m_Image ? m_Image : Texture2D::White());
        m_Material->SetResource<Sampler>("GE_IMAGE_SAMPLER", m_Sampler);
        command->Render(m_Pipeline, 0);
    }
}
