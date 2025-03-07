#include "GEpch.h"
#include "VulkanRendererAPI.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"
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
        vkCmdDrawIndexed(VulkanContext::Get()->GetCurrentCommandBuffer(), indexCount, 1, 0, 0, 0);
    }
    void VulkanRendererAPI::DrawLines(uint32_t indexCount)
    {
        vkCmdDrawIndexed(VulkanContext::Get()->GetCurrentCommandBuffer(), indexCount, 1, 0, 0, 0);
    }
    void VulkanRendererAPI::DrawPoints(uint32_t indexCount)
    {
        vkCmdDrawIndexed(VulkanContext::Get()->GetCurrentCommandBuffer(), indexCount, 1, 0, 0, 0);
    }
    void VulkanRendererAPI::EnableDepthWrite(bool enabled)
    {
    }
    void VulkanRendererAPI::SetCull(CullMode mode)
    {
    }
    void VulkanRendererAPI::SetBlend(BlendMode modeColor, BlendMode modeAlpha, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha)
    {
    }
    void VulkanRendererAPI::DrawTrianglesInstance(uint32_t indexCount, uint32_t instanceCount)
    {
        vkCmdDrawIndexed(VulkanContext::Get()->GetCurrentCommandBuffer(), indexCount, instanceCount, 0, 0, 0);
    }
    void VulkanRendererAPI::DrawTriangles(CommandBuffer* buffer, uint32_t indexCount)
    {
        vkCmdDrawIndexed(static_cast<VulkanCommandBuffer*>(buffer)->GetCommandBuffer(), indexCount, 1, 0, 0, 0);
    }
    void VulkanRendererAPI::DrawLines(CommandBuffer* buffer, uint32_t indexCount)
    {
        vkCmdDrawIndexed(static_cast<VulkanCommandBuffer*>(buffer)->GetCommandBuffer(), indexCount, 1, 0, 0, 0);
    }
    void VulkanRendererAPI::DrawPoints(CommandBuffer* buffer, uint32_t indexCount)
    {
        vkCmdDrawIndexed(static_cast<VulkanCommandBuffer*>(buffer)->GetCommandBuffer(), indexCount, 1, 0, 0, 0);
    }
    void VulkanRendererAPI::DrawTrianglesInstance(CommandBuffer* buffer, uint32_t indexCount, uint32_t instanceCount)
    {
        vkCmdDrawIndexed(static_cast<VulkanCommandBuffer*>(buffer)->GetCommandBuffer(), indexCount, instanceCount, 0, 0, 0);
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
    void VulkanRendererAPI::BeginGraphicsCommand()
    {
        
        VkCommandBufferBeginInfo    beginInfo{};
        beginInfo.sType             = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags             = 0; // Optional
        beginInfo.pInheritanceInfo  = nullptr; // Optional
        VulkanContext::Get()->BeginGraphicsCommandBuffer();
        vkResetCommandBuffer(VulkanContext::Get()->GetCurrentCommandBuffer(), 0);
        VK_CHECK_RESULT(vkBeginCommandBuffer(VulkanContext::Get()->GetCurrentCommandBuffer(), &beginInfo));
    }
    void VulkanRendererAPI::EndGraphicsCommand()
    {
        VkCommandBuffer commandBuffer = VulkanContext::Get()->EndGraphicsCommandBuffer();
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
    CommandBuffer* VulkanRendererAPI::BeginGraphicsCommand(Ref<FrameBuffer>& buffer, const Camera& camera)
    {
		VkCommandBufferBeginInfo    beginInfo{};
		beginInfo.sType             = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags             = 0; // Optional
		beginInfo.pInheritanceInfo  = nullptr; // Optional
		VulkanContext::Get()->BeginGraphicsCommandBuffer();
		vkResetCommandBuffer(VulkanContext::Get()->GetCurrentCommandBuffer(), 0);
		VK_CHECK_RESULT(vkBeginCommandBuffer(VulkanContext::Get()->GetCurrentCommandBuffer(), &beginInfo));

        auto cmd = new VulkanCommandBuffer(VulkanContext::Get()->GetCurrentCommandBuffer());
        cmd->Begin(buffer, camera);
        return cmd;
    }
    CommandBuffer* VulkanRendererAPI::BeginGraphicsCommand(Ref<FrameBuffer>& buffer, const Editor::EditorCamera& camera)
    {
		VkCommandBufferBeginInfo    beginInfo{};
		beginInfo.sType             = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags             = 0; // Optional
		beginInfo.pInheritanceInfo  = nullptr; // Optional
		VulkanContext::Get()->BeginGraphicsCommandBuffer();
		vkResetCommandBuffer(VulkanContext::Get()->GetCurrentCommandBuffer(), 0);
		VK_CHECK_RESULT(vkBeginCommandBuffer(VulkanContext::Get()->GetCurrentCommandBuffer(), &beginInfo));

		auto cmd = new VulkanCommandBuffer(VulkanContext::Get()->GetCurrentCommandBuffer());
		cmd->Begin(buffer, camera);
		return cmd;
    }
    void VulkanRendererAPI::EndGraphicsCommand(CommandBuffer* buffer)
    {
        VkCommandBuffer cmd = ((VulkanCommandBuffer*)buffer)->GetCommandBuffer();
        buffer->End();

		VK_CHECK_RESULT(vkEndCommandBuffer(cmd));

        std::vector<VkSemaphore> waitSemaphores;
        auto waitCmds = buffer->GetWaitCommands();
        for (auto it = waitCmds.begin(); it != waitCmds.end(); it++)
        {
            waitSemaphores.push_back(((VulkanCommandBuffer*)*it)->GetSemaphore());
        }
		std::vector<VkPipelineStageFlags> waitStages(waitSemaphores.size(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		VkSemaphore signalSemaphores[] = { ((VulkanCommandBuffer*)buffer)->GetSemaphore() };

		VkSubmitInfo                    submitInfo{};
		submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount   = 1;
		submitInfo.pCommandBuffers      = &cmd;
		submitInfo.waitSemaphoreCount   = waitSemaphores.size();
		submitInfo.pWaitSemaphores      = waitSemaphores.data();
		submitInfo.pWaitDstStageMask    = waitStages.data();
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores    = signalSemaphores;

		VK_CHECK_RESULT(vkQueueSubmit(VulkanContext::Get()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));

        delete buffer;
        buffer = nullptr;
    }
    void VulkanRendererAPI::BeginSecondaryCommand()
    {
		VkCommandBufferBeginInfo    beginInfo{};
		beginInfo.sType             = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags             = 0; // Optional
		beginInfo.pInheritanceInfo  = nullptr; // Optional
		VulkanContext::Get()->BeginSecondaryCommandBuffer();
		vkResetCommandBuffer(VulkanContext::Get()->GetCurrentSecondaryCommandBuffer(), 0);
		VK_CHECK_RESULT(vkBeginCommandBuffer(VulkanContext::Get()->GetCurrentSecondaryCommandBuffer(), &beginInfo));
    }
    void VulkanRendererAPI::EndSecondaryCommand()
    {
		VkCommandBuffer commandBuffer = VulkanContext::Get()->EndSecondaryCommandBuffer();
		VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

        vkCmdExecuteCommands(VulkanContext::Get()->GetCurrentCommandBuffer(), 1, &commandBuffer);
    }
    float VulkanRendererAPI::GetTime()
    {
        return (float)glfwGetTime();
    }

    std::vector<std::string> VulkanRendererAPI::GetExtensions()
    {
        std::vector<std::string> ext;
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(VulkanContext::Get()->GetPhysicalDevice(), nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(VulkanContext::Get()->GetPhysicalDevice(), nullptr, &extensionCount, availableExtensions.data());

        for (const auto& extension : availableExtensions)
        {
            ext.push_back(extension.extensionName);
        }
        return ext;
    }

    uint32_t VulkanRendererAPI::GetMaxTextureSize()
    {
		VkPhysicalDeviceProperties      deviceProperties;
		vkGetPhysicalDeviceProperties(VulkanContext::Get()->GetPhysicalDevice(), &deviceProperties);
        return deviceProperties.limits.maxImageDimension2D;
    }

    uint32_t VulkanRendererAPI::GetMaxCombinedTextureCount()
    {
		VkPhysicalDeviceProperties      deviceProperties;
		vkGetPhysicalDeviceProperties(VulkanContext::Get()->GetPhysicalDevice(), &deviceProperties);
		return deviceProperties.limits.maxDescriptorSetSampledImages;
    }

    uint32_t VulkanRendererAPI::GetMaxPerStageTextureCount()
    {
		VkPhysicalDeviceProperties      deviceProperties;
		vkGetPhysicalDeviceProperties(VulkanContext::Get()->GetPhysicalDevice(), &deviceProperties);
		return deviceProperties.limits.maxPerStageDescriptorSampledImages;
    }

    uint32_t VulkanRendererAPI::GetMaxTextureArrayLayers()
    {
		VkPhysicalDeviceProperties      deviceProperties;
		vkGetPhysicalDeviceProperties(VulkanContext::Get()->GetPhysicalDevice(), &deviceProperties);
		return deviceProperties.limits.maxImageArrayLayers;
    }

    uint32_t VulkanRendererAPI::GetMinUniformBufferOffsetAlignment()
    {
		VkPhysicalDeviceProperties      deviceProperties;
		vkGetPhysicalDeviceProperties(VulkanContext::Get()->GetPhysicalDevice(), &deviceProperties);
		return deviceProperties.limits.minUniformBufferOffsetAlignment;
    }

    uint32_t VulkanRendererAPI::GetMaxUniformBufferSize()
    {
		VkPhysicalDeviceProperties      deviceProperties;
		vkGetPhysicalDeviceProperties(VulkanContext::Get()->GetPhysicalDevice(), &deviceProperties);
		return deviceProperties.limits.maxUniformBufferRange;
    }

    Vector3 VulkanRendererAPI::GetMaxComputeWorkGroupCount()
    {
		VkPhysicalDeviceProperties      deviceProperties;
		vkGetPhysicalDeviceProperties(VulkanContext::Get()->GetPhysicalDevice(), &deviceProperties);
        int x = deviceProperties.limits.maxComputeWorkGroupCount[0];
        int y = deviceProperties.limits.maxComputeWorkGroupCount[1];
        int z = deviceProperties.limits.maxComputeWorkGroupCount[2];
        return Vector3(x, y, z);
    }

    Vector3 VulkanRendererAPI::GetMaxComputeWorkGroupSize()
    {
		VkPhysicalDeviceProperties      deviceProperties;
		vkGetPhysicalDeviceProperties(VulkanContext::Get()->GetPhysicalDevice(), &deviceProperties);
		int x = deviceProperties.limits.maxComputeWorkGroupSize[0];
		int y = deviceProperties.limits.maxComputeWorkGroupSize[1];
		int z = deviceProperties.limits.maxComputeWorkGroupSize[2];
		return Vector3(x, y, z);
    }

    uint32_t VulkanRendererAPI::GetMaxComputeWorkGroupInvocations()
    {
		VkPhysicalDeviceProperties      deviceProperties;
		vkGetPhysicalDeviceProperties(VulkanContext::Get()->GetPhysicalDevice(), &deviceProperties);
        return deviceProperties.limits.maxComputeWorkGroupInvocations;
    }

    void VulkanRendererAPI::BeginComputeCommand()
    {
		VkCommandBufferBeginInfo    beginInfo{};
		beginInfo.sType             = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags             = 0; // Optional
		beginInfo.pInheritanceInfo  = nullptr; // Optional
		VulkanContext::Get()->BeginComputeCommandBuffer();
		vkResetCommandBuffer(VulkanContext::Get()->GetCurrentCommandBuffer(), 0);
		VK_CHECK_RESULT(vkBeginCommandBuffer(VulkanContext::Get()->GetCurrentCommandBuffer(), &beginInfo));
    }

    void VulkanRendererAPI::EndComputeCommand()
    {
		VkCommandBuffer commandBuffer = VulkanContext::Get()->EndComputeCommandBuffer();
		VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

		VkSemaphore waitSemaphores[] = { VulkanContext::Get()->GetCurrentSemaphore() };
		VulkanContext::Get()->MoveToNextSemaphore();
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT };
		VkSemaphore signalSemaphores[] = { VulkanContext::Get()->GetCurrentSemaphore() };

		VkSubmitInfo                    submitInfo{};
		submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount   = 1;
		submitInfo.pCommandBuffers      = &commandBuffer;
		submitInfo.waitSemaphoreCount   = 1;
		submitInfo.pWaitSemaphores      = waitSemaphores;
        submitInfo.pWaitDstStageMask    = waitStages;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores    = signalSemaphores;

		VK_CHECK_RESULT(vkQueueSubmit(VulkanContext::Get()->GetComputeQueue(), 1, &submitInfo, VK_NULL_HANDLE));
    }

    void VulkanRendererAPI::Compute(const uint32_t x, const uint32_t y, const uint32_t z)
    {
        vkCmdDispatch(VulkanContext::Get()->GetCurrentCommandBuffer(), x, y, z);
    }

    void VulkanRendererAPI::Compute(CommandBuffer* buffer, const uint32_t x, const uint32_t y, const uint32_t z)
    {
        vkCmdDispatch(static_cast<VulkanCommandBuffer*>(buffer)->GetCommandBuffer(), x, y, z);
    }

}
