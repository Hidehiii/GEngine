#include "GEpch.h"
#include "VulkanRendererAPI.h"
#include "Platform/Vulkan/VulkanUtils.h"
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
        VulkanContext::Get()->SetClearColor(color);
    }
    void VulkanRendererAPI::Clear()
    {
    }
    void VulkanRendererAPI::DrawTriangles(uint32_t indexCount)
    {
        vkCmdDrawIndexed(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), indexCount, 1, 0, 0, 0);
    }
    void VulkanRendererAPI::DrawLines(uint32_t indexCount)
    {
        vkCmdDrawIndexed(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), indexCount, 1, 0, 0, 0);
    }
    void VulkanRendererAPI::DrawPoints(uint32_t indexCount)
    {
        vkCmdDrawIndexed(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), indexCount, 1, 0, 0, 0);
    }
    void VulkanRendererAPI::EnableDepthWrite(bool enabled)
    {
    }
    void VulkanRendererAPI::EnableDepthTest(bool enabled)
    {
    }
    void VulkanRendererAPI::SetCull(MaterialCullMode mode)
    {
    }
    void VulkanRendererAPI::SetBlend(MaterialBlendMode mode, uint32_t source, uint32_t dest)
    {
    }
    void VulkanRendererAPI::DrawTrianglesInstance(uint32_t indexCount, uint32_t instanceCount)
    {
        vkCmdDrawIndexed(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), indexCount, instanceCount, 0, 0, 0);
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
    void VulkanRendererAPI::BeginDrawCommand()
    {
        
        VkCommandBufferBeginInfo    beginInfo{};
        beginInfo.sType             = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags             = 0; // Optional
        beginInfo.pInheritanceInfo  = nullptr; // Optional
        VulkanContext::Get()->BeginDrawCommandBuffer();
        vkResetCommandBuffer(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), 0);
        VK_CHECK_RESULT(vkBeginCommandBuffer(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), &beginInfo));
    }
    void VulkanRendererAPI::EndDrawCommand()
    {
        VkCommandBuffer commandBuffer = VulkanContext::Get()->EndDrawCommandBuffer();
        VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

        VkSemaphore waitSemaphores[] = { VulkanContext::Get()->GetCurrentSemaphore()};
        VulkanContext::Get()->MoveToNextSemaphore();
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSemaphore signalSemaphores[] = { VulkanContext::Get()->GetCurrentSemaphore() };

        VkSubmitInfo    submitInfo{};
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = &commandBuffer;
        submitInfo.waitSemaphoreCount   = 1;
        submitInfo.pWaitSemaphores      = waitSemaphores;
        submitInfo.pWaitDstStageMask    = waitStages;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphores;

		VK_CHECK_RESULT(vkQueueSubmit(VulkanContext::Get()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
    }
    float VulkanRendererAPI::GetTime()
    {
        return 0.0f;
    }

}
