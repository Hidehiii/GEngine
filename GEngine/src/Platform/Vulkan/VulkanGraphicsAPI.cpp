#include "GEpch.h"
#include "Platform/Vulkan/VulkanTexture2DCombineSampler.h"
#include "Platform/Vulkan/VulkanCubeMapCombineSampler.h"
#include "Platform/Vulkan/VulkanCubeMap.h"
#include "Platform/Vulkan/VulkanTexture2DArray.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/VulkanGraphicsPipeline.h"
#include "Platform/Vulkan/VulkanComputePipeline.h"
#include "Platform/Vulkan/VulkanSampler.h"
#include "Platform/Vulkan/VulkanStorageBuffer.h"
#include "Platform/Vulkan/VulkanStorageImage2D.h"
#include "Platform/Vulkan/VulkanGraphicsAPI.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanGraphicsPresent.h"
#include "Platform/Vulkan/VulkanRenderPass.h"
#include "Platform/Vulkan/VulkanFrameBuffer.h"
#include "Platform/Vulkan/VulkanTexture2D.h"
#include "Platform/Vulkan/VulkanStorageImage2D.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"
#include "Platform/Vulkan/VulkanVertexBuffer.h"
#include <algorithm>
#include <set>
#include <utility>

namespace
{
	struct VulkanResourceState
	{
		VkImageLayout Layout;
		VkAccessFlags Access;
		VkPipelineStageFlags Stage;
	};

	VulkanResourceState ToVulkanResourceState(GEngine::GraphicsResourceState state)
	{
		using State = GEngine::GraphicsResourceState;
		switch (state)
		{
		case State::RenderTarget: return { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		case State::DepthWrite: return { VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT };
		case State::ShaderRead: return { VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT };
		case State::ShaderWrite: return { VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT };
		case State::CopySource: return { VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT };
		case State::CopyDestination: return { VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT };
		case State::Present: return { VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT };
		case State::Undefined: return { VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
		}
		return { VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
	}
}

namespace GEngine
{
	Ref<Texture2DCombineSampler> VulkanGraphicsAPI::CreateTexture2DCombineSampler(const Ref<Texture2D>& texture, const Ref<Sampler>& sampler)
	{
		return CreateRef<VulkanTexture2DCombineSampler>(texture, sampler);
	}
	Ref<CubeMapCombineSampler> VulkanGraphicsAPI::CreateCubeMapCombineSampler(const Ref<CubeMap>& cubemap, const Ref<Sampler>& sampler)
	{
		return CreateRef<VulkanCubeMapCombineSampler>(cubemap, sampler);
	}
	Ref<CubeMap> VulkanGraphicsAPI::CreateCubeMap(uint32_t width, uint32_t height, bool generateMipmap, RenderImage2DFormat format)
	{
		return CreateRef<VulkanCubeMap>(width, height, generateMipmap, format);
	}
	Ref<CubeMap> VulkanGraphicsAPI::CreateCubeMap(const std::string& rightPath, const std::string& leftPath, const std::string& topPath, const std::string& buttomPath, const std::string& backPath, const std::string& frontPath, bool generateMipmap)
	{
		return CreateRef<VulkanCubeMap>(rightPath, leftPath, topPath, buttomPath, backPath, frontPath, generateMipmap);
	}
	Ref<Texture2DArray> VulkanGraphicsAPI::CreateTexture2DArray(uint32_t width, uint32_t height, uint32_t layers, RenderImage2DFormat format)
	{
		return CreateRef<VulkanTexture2DArray>(width, height, layers, format);
	}
	Ref<Shader> VulkanGraphicsAPI::CreateShader(const std::string& path)
	{
		return CreateRef<VulkanShader>(path);
	}
	Ref<Material> VulkanGraphicsAPI::CreateMaterial(const Ref<Shader>& shader, const std::string& name)
	{
		return CreateRef<VulkanMaterial>(shader, name);
	}
	Ref<GraphicsPipeline> VulkanGraphicsAPI::CreateGraphicsPipeline(const Ref<Material>& material, const Ref<VertexBuffer>& vertices)
	{
		return CreateRef<VulkanGraphicsPipeline>(material, vertices);
	}
	Ref<ComputePipeline> VulkanGraphicsAPI::CreateComputePipeline(const Ref<Material>& material)
	{
		return CreateRef<VulkanComputePipeline>(material);
	}
	Ref<Sampler> VulkanGraphicsAPI::CreateSampler(const SamplerSpecification& specification)
	{
		return CreateRef<VulkanSampler>(specification);
	}
	Ref<StorageBuffer> VulkanGraphicsAPI::CreateStorageBuffer(uint32_t size)
	{
		return CreateRef<VulkanStorageBuffer>(size);
	}
	Ref<StorageImage2D> VulkanGraphicsAPI::CreateStorageImage2D(uint32_t width, uint32_t height, ComputeImage2DFormat format)
	{
		return CreateRef<VulkanStorageImage2D>(width, height, format);
	}
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
        const VkPhysicalDevice physicalDevice = VulkanContext::Get()->GetPhysicalDevice();
        VkPhysicalDeviceProperties deviceProperties{};
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        VkPhysicalDeviceFeatures deviceFeatures{};
        vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());
        std::set<std::string> extensions;
        for (const auto& extension : availableExtensions)
            extensions.insert(extension.extensionName);

        VkFormatProperties colorFormatProperties{};
        vkGetPhysicalDeviceFormatProperties(physicalDevice, VK_FORMAT_R8G8B8A8_UNORM, &colorFormatProperties);
        const bool supportsSampledColor =
            (colorFormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) != 0;

        VkFormatProperties storageImageFormatProperties{};
        vkGetPhysicalDeviceFormatProperties(
            physicalDevice, VK_FORMAT_R32G32B32A32_SFLOAT, &storageImageFormatProperties);
        const bool supportsStorageImage =
            (storageImageFormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) != 0;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
        const bool supportsComputeQueue = std::any_of(queueFamilies.begin(), queueFamilies.end(),
            [](const VkQueueFamilyProperties& queueFamily)
            {
                return (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0;
            });

        const bool supportsRenderPass2 = deviceProperties.apiVersion >= VK_API_VERSION_1_2 ||
            extensions.find(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME) != extensions.end();

        capabilities.Backend = "Vulkan";
        capabilities.Version = std::to_string(VK_VERSION_MAJOR(deviceProperties.apiVersion)) + "." +
            std::to_string(VK_VERSION_MINOR(deviceProperties.apiVersion)) + "." +
            std::to_string(VK_VERSION_PATCH(deviceProperties.apiVersion));
        capabilities.QuerySources = {
            "VkPhysicalDeviceProperties.apiVersion=" + capabilities.Version,
            "device extensions=" + std::to_string(extensionCount),
            "VK_FORMAT_R8G8B8A8_UNORM SAMPLED_IMAGE_BIT=" + std::to_string(supportsSampledColor),
            "VK_FORMAT_R32G32B32A32_SFLOAT STORAGE_IMAGE_BIT=" + std::to_string(supportsStorageImage),
            "compute queue family=" + std::to_string(supportsComputeQueue),
            "fragmentStoresAndAtomics=" + std::to_string(deviceFeatures.fragmentStoresAndAtomics != 0),
            "render pass 2=Vulkan 1.2 core or VK_KHR_create_renderpass2"
        };

        capabilities.RenderPass = supportsRenderPass2;
        capabilities.FrameBuffer = supportsRenderPass2;
        capabilities.Texture2D = supportsSampledColor;
        capabilities.Texture2DArray = supportsSampledColor && deviceProperties.limits.maxImageArrayLayers > 1;
        capabilities.CubeMap = supportsSampledColor && deviceProperties.limits.maxImageArrayLayers >= 6;
        capabilities.Sampler = true;
        capabilities.UniformBuffer = true;
        capabilities.StorageBuffer = deviceFeatures.fragmentStoresAndAtomics != 0;
        capabilities.StorageImage = supportsStorageImage;
        capabilities.Compute = supportsComputeQueue;
        capabilities.Subpasses = supportsRenderPass2;
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
		if (!std::dynamic_pointer_cast<VulkanCommandBuffer>(first) ||
			!std::dynamic_pointer_cast<VulkanCommandBuffer>(second) || first == second)
			throw std::invalid_argument("Vulkan dependencies require distinct Vulkan command buffers.");
		VkSemaphore s = VK_NULL_HANDLE;
		VkSemaphoreCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		if (vkCreateSemaphore(VulkanContext::Get()->GetDevice(), &info, nullptr, &s) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate a Vulkan dependency semaphore.");
		GE_CORE_ASSERT(std::dynamic_pointer_cast<VulkanCommandBuffer>(first), "Vulkan barriers require Vulkan command buffers.");
		GE_CORE_ASSERT(std::dynamic_pointer_cast<VulkanCommandBuffer>(second), "Vulkan barriers require Vulkan command buffers.");
		m_SubmissionSynchronizations[first.get()].SignalSemaphores.push_back(s);
		m_SubmissionSynchronizations[second.get()].WaitSemaphores.push_back(s);
    }

	void VulkanGraphicsAPI::SubmitCommandBuffer(const Ref<CommandBuffer>& commandBuffer)
	{
		auto vulkanCommandBuffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(commandBuffer);
		GE_CORE_ASSERT(vulkanCommandBuffer, "Vulkan submission requires a Vulkan command buffer.");

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		VkQueue queue = VK_NULL_HANDLE;
		switch (vulkanCommandBuffer->GetType())
		{
		case COMMAND_BUFFER_TYPE_GRAPHICS:
			queue = VulkanContext::Get()->GetGraphicsQueue();
			waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		case COMMAND_BUFFER_TYPE_COMPUTE:
			queue = VulkanContext::Get()->GetComputeQueue();
			waitStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			break;
		case COMMAND_BUFFER_TYPE_TRANSFER:
			queue = VulkanContext::Get()->GetTransferQueue();
			waitStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		default:
			GE_CORE_ASSERT(false, "Vulkan command buffer type is invalid.");
			return;
		}

		auto synchronization = TakeSubmissionSynchronization(commandBuffer.get());
		const auto& waits = synchronization.WaitSemaphores;
		const auto& signals = synchronization.SignalSemaphores;
		std::vector<VkPipelineStageFlags> waitStages(waits.size(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
		VkCommandBuffer nativeCommandBuffer = vulkanCommandBuffer->GetCommandBuffer();
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &nativeCommandBuffer;
		submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waits.size());
		submitInfo.pWaitSemaphores = waits.data();
		submitInfo.pWaitDstStageMask = waitStages.data();
		submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signals.size());
		submitInfo.pSignalSemaphores = signals.data();
		VulkanContext::Get()->SubmitTracked(queue, submitInfo);
		for (auto semaphore : waits)
			VulkanContext::Get()->RetireResource([semaphore](VkDevice device) { vkDestroySemaphore(device, semaphore, nullptr); });
	}

	void VulkanGraphicsAPI::SubmitPresentationCommandBuffer(const Ref<CommandBuffer>& commandBuffer, VkSemaphore acquireSemaphore,
		VkSemaphore presentSemaphore, VkFence completionFence)
	{
		auto vulkanCommandBuffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(commandBuffer);
		GE_CORE_ASSERT(vulkanCommandBuffer, "Vulkan presentation submission requires a Vulkan command buffer.");
		GE_CORE_ASSERT(acquireSemaphore != VK_NULL_HANDLE, "Vulkan presentation requires an acquire semaphore.");
		GE_CORE_ASSERT(presentSemaphore != VK_NULL_HANDLE, "Vulkan presentation requires a present semaphore.");

		auto synchronization = TakeSubmissionSynchronization(commandBuffer.get());
		synchronization.WaitSemaphores.push_back(acquireSemaphore);
		synchronization.SignalSemaphores.push_back(presentSemaphore);
		std::vector<VkPipelineStageFlags> waitStages(synchronization.WaitSemaphores.size(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
		VkCommandBuffer nativeCommandBuffer = vulkanCommandBuffer->GetCommandBuffer();
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &nativeCommandBuffer;
		submitInfo.waitSemaphoreCount = static_cast<uint32_t>(synchronization.WaitSemaphores.size());
		submitInfo.pWaitSemaphores = synchronization.WaitSemaphores.data();
		submitInfo.pWaitDstStageMask = waitStages.data();
		submitInfo.signalSemaphoreCount = static_cast<uint32_t>(synchronization.SignalSemaphores.size());
		submitInfo.pSignalSemaphores = synchronization.SignalSemaphores.data();
		VulkanContext::Get()->SubmitTracked(VulkanContext::Get()->GetGraphicsQueue(), submitInfo, completionFence);
		for (auto semaphore : synchronization.WaitSemaphores)
			if (semaphore != acquireSemaphore)
				VulkanContext::Get()->RetireResource([semaphore](VkDevice device) { vkDestroySemaphore(device, semaphore, nullptr); });
	}

	VulkanGraphicsAPI::SubmissionSynchronization VulkanGraphicsAPI::TakeSubmissionSynchronization(const CommandBuffer* commandBuffer)
	{
		auto synchronization = m_SubmissionSynchronizations.find(commandBuffer);
		if (synchronization == m_SubmissionSynchronizations.end())
			return {};

		auto result = std::move(synchronization->second);
		m_SubmissionSynchronizations.erase(synchronization);
		return result;
	}

	void VulkanGraphicsAPI::WaitForIdle()
	{
		VulkanContext::Get()->WaitForIdle();
	}

	void VulkanGraphicsAPI::TransitionResource(const Ref<CommandBuffer>& commandBuffer, const Ref<GraphicsResource>& resource,
		GraphicsResourceState before, GraphicsResourceState after)
	{
		const auto nativeResource = GetNativeResource(resource);
		if (!resource || nativeResource == nullptr)
			return;

		auto vulkanCommandBuffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(commandBuffer);
		GE_CORE_ASSERT(vulkanCommandBuffer, "Vulkan resource transitions require a Vulkan command buffer.");
		if (before == after)
		{
			if (before == GraphicsResourceState::ShaderWrite)
			{
				VkMemoryBarrier barrier{};
				barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
				barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
				vkCmdPipelineBarrier(vulkanCommandBuffer->GetCommandBuffer(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);
			}
			return;
		}
		const auto source = ToVulkanResourceState(before);
		const auto destination = ToVulkanResourceState(after);
		if (auto texture = std::dynamic_pointer_cast<VulkanTexture2D>(resource))
		{
			texture->SetImageLayout(vulkanCommandBuffer->GetCommandBuffer(), destination.Layout);
			return;
		}
		if (auto image = std::dynamic_pointer_cast<VulkanStorageImage2D>(resource))
		{
			image->SetImageLayout(vulkanCommandBuffer->GetCommandBuffer(), destination.Layout);
			return;
		}

		if (resource->GetResourceType() == GraphicsResourceType::Buffer)
		{
			VkBufferMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barrier.srcAccessMask = source.Access;
			barrier.dstAccessMask = destination.Access;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.buffer = reinterpret_cast<VkBuffer>(nativeResource);
			barrier.offset = 0;
			barrier.size = VK_WHOLE_SIZE;
			vkCmdPipelineBarrier(vulkanCommandBuffer->GetCommandBuffer(), source.Stage, destination.Stage, 0,
				0, nullptr, 1, &barrier, 0, nullptr);
			return;
		}

		GE_CORE_ASSERT(resource->GetResourceType() == GraphicsResourceType::Texture, "Vulkan render-graph resources must declare their type.");
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.srcAccessMask = source.Access;
		barrier.dstAccessMask = destination.Access;
		barrier.oldLayout = source.Layout;
		barrier.newLayout = destination.Layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = reinterpret_cast<VkImage>(nativeResource);
		barrier.subresourceRange.aspectMask = (before == GraphicsResourceState::DepthWrite || after == GraphicsResourceState::DepthWrite)
			? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
		vkCmdPipelineBarrier(vulkanCommandBuffer->GetCommandBuffer(), source.Stage, destination.Stage, 0,
			0, nullptr, 0, nullptr, 1, &barrier);
	}

}
