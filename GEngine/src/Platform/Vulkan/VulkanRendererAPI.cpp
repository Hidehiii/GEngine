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
    void VulkanRendererAPI::DrawTriangles(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
    {
        uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
        vertexArray->Bind();
        vkCmdDrawIndexed(VulkanContext::Get()->GetCurrentCommandBuffer(), count, 1, 0, 0, 0);
    }
    void VulkanRendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
    {
    }
    void VulkanRendererAPI::DrawPoints(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
    {
    }
    void VulkanRendererAPI::EnableDepthWrite(bool enabled)
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
    void VulkanRendererAPI::BeginCommand()
    {
        VkCommandBufferBeginInfo    beginInfo{};
        beginInfo.sType             = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags             = 0; // Optional
        beginInfo.pInheritanceInfo  = nullptr; // Optional
        VulkanContext::Get()->PushCommandBuffer();
        vkResetCommandBuffer(VulkanContext::Get()->GetCurrentCommandBuffer(), 0);
        VK_CHECK_RESULT(vkBeginCommandBuffer(VulkanContext::Get()->GetCurrentCommandBuffer(), &beginInfo));
    }
    void VulkanRendererAPI::EndCommand()
    {
        VkCommandBuffer commandBuffer = VulkanContext::Get()->GetCurrentCommandBuffer();
        VK_CHECK_RESULT(vkEndCommandBuffer(VulkanContext::Get()->PopCommandBuffer()));

        VkSemaphore waitSemaphores[] = { VulkanContext::Get()->GetImageAvailableSemaphores()[0] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSemaphore signalSemaphores[] = { VulkanContext::Get()->GetRenderFinishedSemaphores()[0] };

        VkSubmitInfo    submitInfo{};
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = &commandBuffer;
        submitInfo.waitSemaphoreCount   = 1;
        submitInfo.pWaitSemaphores      = waitSemaphores;
        submitInfo.pWaitDstStageMask    = waitStages;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphores;

		VK_CHECK_RESULT(vkQueueSubmit(VulkanContext::Get()->GetGraphicsQueue(), 1, &submitInfo, VulkanContext::Get()->GetInFlightFences()[0]));
    }
    float VulkanRendererAPI::GetTime()
    {
        return 0.0f;
    }

}
