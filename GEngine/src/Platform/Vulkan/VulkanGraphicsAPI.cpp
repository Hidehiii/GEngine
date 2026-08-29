#include "GEpch.h"
#include "VulkanGraphicsAPI.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanGraphicsPresent.h"
#include "Platform/Vulkan/VulkanRenderPass.h"
#include "Platform/Vulkan/VulkanFrameBuffer.h"
#include "Platform/Vulkan/VulkanTexture2D.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"
#include "Platform/Vulkan/VulkanVertexBuffer.h"
#include <set>

namespace GEngine
{
    VulkanGraphicsAPI::VulkanGraphicsAPI()
    {
        s_API = GRAPHICS_API_VULKAN;
    }
    VulkanGraphicsAPI::~VulkanGraphicsAPI()
    {
    }


    Ref<CommandBuffer> VulkanGraphicsAPI::GetGraphicsCommandBuffer()
    {
        return VulkanContext::Get()->GetCommandBuffer(COMMAND_BUFFER_TYPE_GRAPHICS);
    }

    Ref<CommandBuffer> VulkanGraphicsAPI::GetComputeCommandBuffer()
    {
        return VulkanContext::Get()->GetCommandBuffer(COMMAND_BUFFER_TYPE_COMPUTE);
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

    GraphicsCapabilities VulkanGraphicsAPI::GetCapabilities() const
    {
        GraphicsCapabilities capabilities;
        capabilities.RenderPass = true;
        capabilities.FrameBuffer = true;
        capabilities.Texture2D = true;
        capabilities.Texture2DArray = true;
        capabilities.CubeMap = true;
        capabilities.Sampler = true;
        capabilities.UniformBuffer = true;
        capabilities.StorageBuffer = true;
        capabilities.StorageImage = true;
        capabilities.Compute = true;
        capabilities.Subpasses = true;
        return capabilities;
    }
    Ref<RenderPass> VulkanGraphicsAPI::CreateRenderPass(const RenderPassSpecification& spec)
    {
        return CreateRef<VulkanRenderPass>(spec);
    }
    Ref<FrameBuffer> VulkanGraphicsAPI::CreateFrameBuffer(const Ref<RenderPass>& renderPass, uint32_t width, uint32_t height)
    {
        return CreateRef<VulkanFrameBuffer>(renderPass, width, height);
    }
    Ref<FrameBuffer> VulkanGraphicsAPI::ResizeFrameBuffer(const Ref<FrameBuffer>& buffer, uint32_t width, uint32_t height)
    {
        return CreateRef<VulkanFrameBuffer>(buffer, width, height);
    }
    Ref<Texture2D> VulkanGraphicsAPI::CreateTexture2D(uint32_t width, uint32_t height, RenderImage2DFormat format)
    {
        return CreateRef<VulkanTexture2D>(width, height, format);
    }
    Ref<Texture2D> VulkanGraphicsAPI::CreateTexture2D(const std::string& path)
    {
        return CreateRef<VulkanTexture2D>(path);
    }
    Ref<Texture2D> VulkanGraphicsAPI::CreateTexture2D(uint32_t width, uint32_t height, void* data, uint32_t size, RenderImage2DFormat format)
    {
        return CreateRef<VulkanTexture2D>(width, height, data, size, format);
    }
    Ref<UniformBuffer> VulkanGraphicsAPI::CreateUniformBuffer(uint32_t size, uint32_t count, bool autoSetDataDynamic)
    {
        return CreateRef<VulkanUniformBuffer>(size, count, autoSetDataDynamic);
    }
    Ref<VertexBuffer> VulkanGraphicsAPI::CreateVertexBuffer(uint32_t size, uint32_t sizeInstance, VertexTopology type)
    {
        return CreateRef<VulkanVertexBuffer>(size, sizeInstance, type);
    }
    Ref<VertexBuffer> VulkanGraphicsAPI::CreateVertexBuffer(const void* vertices, uint32_t size, uint32_t sizeInstance, VertexTopology type)
    {
        return CreateRef<VulkanVertexBuffer>(vertices, size, sizeInstance, type);
    }
    Ref<IndexBuffer> VulkanGraphicsAPI::CreateIndexBuffer(const uint32_t* indices, uint32_t count)
    {
        return CreateRef<VulkanIndexBuffer>(indices, count);
    }

    uint32_t VulkanGraphicsAPI::GetMaxTexture2DSize()
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

}
