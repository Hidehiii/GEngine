#include "GEpch.h"
#include "VulkanRendererAPI.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanGraphicsPresent.h"
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
    void VulkanRendererAPI::EnableDepthWrite(bool enabled)
    {
    }
    void VulkanRendererAPI::SetCull(CullMode mode)
    {
    }
    void VulkanRendererAPI::SetBlend(BlendMode modeColor, BlendMode modeAlpha, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha)
    {
    }
    void VulkanRendererAPI::DrawTriangles(CommandBuffer* buffer, uint32_t indexCount)
    {
        vkCmdDrawIndexed(((VulkanCommandBuffer*)buffer)->GetCommandBuffer(), indexCount, 1, 0, 0, 0);
    }
    void VulkanRendererAPI::DrawLines(CommandBuffer* buffer, uint32_t indexCount)
    {
        vkCmdDrawIndexed(((VulkanCommandBuffer*)buffer)->GetCommandBuffer(), indexCount, 1, 0, 0, 0);
    }
    void VulkanRendererAPI::DrawPoints(CommandBuffer* buffer, uint32_t indexCount)
    {
        vkCmdDrawIndexed(((VulkanCommandBuffer*)buffer)->GetCommandBuffer(), indexCount, 1, 0, 0, 0);
    }
    void VulkanRendererAPI::DrawTrianglesInstance(CommandBuffer* buffer, uint32_t indexCount, uint32_t instanceCount)
    {
        vkCmdDrawIndexed(((VulkanCommandBuffer*)buffer)->GetCommandBuffer(), indexCount, instanceCount, 0, 0, 0);
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

    Ref<CommandBuffer> VulkanRendererAPI::GetGraphicsCommandBuffer()
    {
        return VulkanContext::Get()->GetCommandBuffer(CommandBufferType::Graphics);
    }

    Ref<CommandBuffer> VulkanRendererAPI::GetComputeCommandBuffer()
    {
        return VulkanContext::Get()->GetCommandBuffer(CommandBufferType::Compute);
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

    void VulkanRendererAPI::RegisterSynchronousCommands(Ref<CommandBuffer>& first, Ref<CommandBuffer>& second)
    {
        VkSemaphore s = VulkanContext::Get()->GetSemaphore();
        std::dynamic_pointer_cast<VulkanCommandBuffer>(first)->AddSignalSemaphore(s);
        std::dynamic_pointer_cast<VulkanCommandBuffer>(second)->AddWaitSemaphore(s);
    }

    void VulkanRendererAPI::Compute(CommandBuffer* buffer, const uint32_t x, const uint32_t y, const uint32_t z)
    {
        vkCmdDispatch(((VulkanCommandBuffer*)buffer)->GetCommandBuffer(), x, y, z);
    }

}
