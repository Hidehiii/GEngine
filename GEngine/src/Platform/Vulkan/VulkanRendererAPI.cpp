#include "GEpch.h"
#include "VulkanRendererAPI.h"
#include "Platform/Vulkan/VulkanContext.h"
#include <set>

namespace GEngine
{


    void VulkanRendererAPI::Init()
    {
		
    }
    void VulkanRendererAPI::Uninit()
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
    void VulkanRendererAPI::EnableDepthMask(bool enabled)
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
    void VulkanRendererAPI::BeginCommand(int index)
    {
        VkCommandBufferBeginInfo    beginInfo{};
        beginInfo.sType             = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags             = 0; // Optional
        beginInfo.pInheritanceInfo  = nullptr; // Optional
        if (vkBeginCommandBuffer(VulkanContext::GetCommandBubffers().GetCommandBuffer(index), &beginInfo) != VK_SUCCESS) {
            GE_CORE_ERROR("failed to begin recording command buffer!");
        }
    }
    void VulkanRendererAPI::EndCommand(int index)
    {
        if (vkEndCommandBuffer(VulkanContext::GetCommandBubffers().GetCommandBuffer(index)) != VK_SUCCESS) {
            GE_CORE_ERROR("failed to record command buffer!");
        }
    }
    float VulkanRendererAPI::GetTime()
    {
        return 0.0f;
    }

}
