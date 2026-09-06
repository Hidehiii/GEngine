#include "GEpch.h"
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
#include <set>

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

		const auto& waits = vulkanCommandBuffer->GetWaitSemaphores();
		const auto& signals = vulkanCommandBuffer->GetSignalSemaphores();
		std::vector<VkPipelineStageFlags> waitStages(waits.size(), waitStage);
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
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
		vulkanCommandBuffer->ClearSignalSemaphores();
		vulkanCommandBuffer->ClearWaitSemaphores();
	}

	void VulkanGraphicsAPI::TransitionResource(const Ref<CommandBuffer>& commandBuffer, const Ref<GraphicsResource>& resource,
		GraphicsResourceState before, GraphicsResourceState after)
	{
		const auto nativeResource = GetNativeResource(resource);
		if (!resource || nativeResource == nullptr || before == after)
			return;

		auto vulkanCommandBuffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(commandBuffer);
		GE_CORE_ASSERT(vulkanCommandBuffer, "Vulkan resource transitions require a Vulkan command buffer.");
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
