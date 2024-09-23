#include "GEpch.h"
#include "VulkanRendererAPI.h"

namespace GEngine
{
    void VulkanRendererAPI::Init()
    {
    }
    void VulkanRendererAPI::SetClearColor(const Vector4& color)
    {
    }
    void VulkanRendererAPI::Clear()
    {
    }
    void VulkanRendererAPI::DrawTriangles(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
    {
    }
    void VulkanRendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
    {
    }
    void VulkanRendererAPI::DrawPoints(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
    {
    }
    void VulkanRendererAPI::EnableDepthTest(bool enabled)
    {
    }
    void VulkanRendererAPI::SetCull(Material_CullMode mode)
    {
    }
    void VulkanRendererAPI::SetBlend(Material_BlendMode mode, uint32_t source, uint32_t dest)
    {
    }
    void VulkanRendererAPI::DrawTrianglesInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t instanceCount)
    {
    }
    void VulkanRendererAPI::SetLineWidth(float width)
    {
    }
    void VulkanRendererAPI::SetPointSize(float size)
    {
    }
    void VulkanRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
    }
    float VulkanRendererAPI::GetTime()
    {
        return 0.0f;
    }
}