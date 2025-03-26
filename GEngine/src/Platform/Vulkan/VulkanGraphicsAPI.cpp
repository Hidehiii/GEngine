#include "GEpch.h"
#include "VulkanGraphicsAPI.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanGraphicsPresent.h"
#include <set>

namespace GEngine
{
    VulkanGraphicsAPI::VulkanGraphicsAPI()
    {
        s_API = GraphicsAPI::API::Vulkan;
    }
    VulkanGraphicsAPI::~VulkanGraphicsAPI()
    {
    }
    void VulkanGraphicsAPI::Init()
    {
		
    }
    void VulkanGraphicsAPI::Uninit()
    {

    }
    void VulkanGraphicsAPI::SetClearColor(const Vector4& color)
    {
        VulkanContext::Get()->SetClearColor(color);
    }
    void VulkanGraphicsAPI::Clear()
    {
    }
    void VulkanGraphicsAPI::EnableDepthWrite(bool enabled)
    {
    }
    void VulkanGraphicsAPI::SetCull(CullMode mode)
    {
    }
    void VulkanGraphicsAPI::SetBlend(BlendMode modeColor, BlendMode modeAlpha, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha)
    {
    }
    void VulkanGraphicsAPI::DrawTriangles(CommandBuffer* buffer, uint32_t indexCount)
    {
        vkCmdDrawIndexed(((VulkanCommandBuffer*)buffer)->GetCommandBuffer(), indexCount, 1, 0, 0, 0);
    }
    void VulkanGraphicsAPI::DrawLines(CommandBuffer* buffer, uint32_t indexCount)
    {
        vkCmdDrawIndexed(((VulkanCommandBuffer*)buffer)->GetCommandBuffer(), indexCount, 1, 0, 0, 0);
    }
    void VulkanGraphicsAPI::DrawPoints(CommandBuffer* buffer, uint32_t indexCount)
    {
        vkCmdDrawIndexed(((VulkanCommandBuffer*)buffer)->GetCommandBuffer(), indexCount, 1, 0, 0, 0);
    }
    void VulkanGraphicsAPI::DrawTrianglesInstance(CommandBuffer* buffer, uint32_t indexCount, uint32_t instanceCount)
    {
        vkCmdDrawIndexed(((VulkanCommandBuffer*)buffer)->GetCommandBuffer(), indexCount, instanceCount, 0, 0, 0);
    }
    void VulkanGraphicsAPI::SetLineWidth(float width)
    {
    }
    void VulkanGraphicsAPI::SetPointSize(float size)
    {
    }
    void VulkanGraphicsAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
    }

    Ref<CommandBuffer> VulkanGraphicsAPI::GetGraphicsCommandBuffer()
    {
        return VulkanContext::Get()->GetCommandBuffer(CommandBufferType::Graphics);
    }

    Ref<CommandBuffer> VulkanGraphicsAPI::GetComputeCommandBuffer()
    {
        return VulkanContext::Get()->GetCommandBuffer(CommandBufferType::Compute);
    }

    float VulkanGraphicsAPI::GetTime()
    {
        return (float)glfwGetTime();
    }

    std::vector<std::string> VulkanGraphicsAPI::GetExtensions()
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

    uint32_t VulkanGraphicsAPI::GetMaxTextureSize()
    {
		VkPhysicalDeviceProperties      deviceProperties;
		vkGetPhysicalDeviceProperties(VulkanContext::Get()->GetPhysicalDevice(), &deviceProperties);
        return deviceProperties.limits.maxImageDimension2D;
    }

    uint32_t VulkanGraphicsAPI::GetMaxCombinedTextureCount()
    {
		VkPhysicalDeviceProperties      deviceProperties;
		vkGetPhysicalDeviceProperties(VulkanContext::Get()->GetPhysicalDevice(), &deviceProperties);
		return deviceProperties.limits.maxDescriptorSetSampledImages;
    }

    uint32_t VulkanGraphicsAPI::GetMaxPerStageTextureCount()
    {
		VkPhysicalDeviceProperties      deviceProperties;
		vkGetPhysicalDeviceProperties(VulkanContext::Get()->GetPhysicalDevice(), &deviceProperties);
		return deviceProperties.limits.maxPerStageDescriptorSampledImages;
    }

    uint32_t VulkanGraphicsAPI::GetMaxTextureArrayLayers()
    {
		VkPhysicalDeviceProperties      deviceProperties;
		vkGetPhysicalDeviceProperties(VulkanContext::Get()->GetPhysicalDevice(), &deviceProperties);
		return deviceProperties.limits.maxImageArrayLayers;
    }

    uint32_t VulkanGraphicsAPI::GetMinUniformBufferOffsetAlignment()
    {
		VkPhysicalDeviceProperties      deviceProperties;
		vkGetPhysicalDeviceProperties(VulkanContext::Get()->GetPhysicalDevice(), &deviceProperties);
		return deviceProperties.limits.minUniformBufferOffsetAlignment;
    }

    uint32_t VulkanGraphicsAPI::GetMaxUniformBufferSize()
    {
		VkPhysicalDeviceProperties      deviceProperties;
		vkGetPhysicalDeviceProperties(VulkanContext::Get()->GetPhysicalDevice(), &deviceProperties);
		return deviceProperties.limits.maxUniformBufferRange;
    }

    Vector3 VulkanGraphicsAPI::GetMaxComputeWorkGroupCount()
    {
		VkPhysicalDeviceProperties      deviceProperties;
		vkGetPhysicalDeviceProperties(VulkanContext::Get()->GetPhysicalDevice(), &deviceProperties);
        int x = deviceProperties.limits.maxComputeWorkGroupCount[0];
        int y = deviceProperties.limits.maxComputeWorkGroupCount[1];
        int z = deviceProperties.limits.maxComputeWorkGroupCount[2];
        return Vector3(x, y, z);
    }

    Vector3 VulkanGraphicsAPI::GetMaxComputeWorkGroupSize()
    {
		VkPhysicalDeviceProperties      deviceProperties;
		vkGetPhysicalDeviceProperties(VulkanContext::Get()->GetPhysicalDevice(), &deviceProperties);
		int x = deviceProperties.limits.maxComputeWorkGroupSize[0];
		int y = deviceProperties.limits.maxComputeWorkGroupSize[1];
		int z = deviceProperties.limits.maxComputeWorkGroupSize[2];
		return Vector3(x, y, z);
    }

    uint32_t VulkanGraphicsAPI::GetMaxComputeWorkGroupInvocations()
    {
		VkPhysicalDeviceProperties      deviceProperties;
		vkGetPhysicalDeviceProperties(VulkanContext::Get()->GetPhysicalDevice(), &deviceProperties);
        return deviceProperties.limits.maxComputeWorkGroupInvocations;
    }

    void VulkanGraphicsAPI::SetCommandsBarrier(Ref<CommandBuffer>& first, Ref<CommandBuffer>& second)
    {
        VkSemaphore s = VulkanContext::Get()->GetSemaphore();
        std::dynamic_pointer_cast<VulkanCommandBuffer>(first)->AddSignalSemaphore(s);
        std::dynamic_pointer_cast<VulkanCommandBuffer>(second)->AddWaitSemaphore(s);
    }

    void VulkanGraphicsAPI::Compute(CommandBuffer* buffer, const uint32_t x, const uint32_t y, const uint32_t z)
    {
        vkCmdDispatch(((VulkanCommandBuffer*)buffer)->GetCommandBuffer(), x, y, z);
    }

}
