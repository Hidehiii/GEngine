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

	VkPipelineStageFlags ToVulkanStage(GEngine::GraphicsPipelineStage stage, VkPipelineStageFlags fallback)
	{
		using Stage = GEngine::GraphicsPipelineStage;
		switch (stage)
		{
		case Stage::Graphics: return VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
		case Stage::Compute: return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		case Stage::Transfer: return VK_PIPELINE_STAGE_TRANSFER_BIT;
		case Stage::All: return fallback;
		}
		return fallback;
	}

	VkAccessFlags ToVulkanAccess(GEngine::GraphicsResourceAccess access, VkAccessFlags fallback)
	{
		using Access = GEngine::GraphicsResourceAccess;
		const auto reads = fallback & (VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT);
		const auto writes = fallback & (VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT |
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
		switch (access)
		{
		case Access::Read: return reads;
		case Access::Write: return writes;
		case Access::ReadWrite: return fallback;
		}
		return fallback;
	}

	VkPipelineStageFlags StageForCommandBuffer(VkPipelineStageFlags stage, VkAccessFlags& access,
		GEngine::CommandBufferType type, bool source)
	{
		const VkPipelineStageFlags graphicsStages = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT |
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		if (type == GEngine::COMMAND_BUFFER_TYPE_GRAPHICS)
			stage &= ~VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		if (type == GEngine::COMMAND_BUFFER_TYPE_COMPUTE || type == GEngine::COMMAND_BUFFER_TYPE_TRANSFER)
			stage &= ~graphicsStages;
		if (type == GEngine::COMMAND_BUFFER_TYPE_TRANSFER)
			stage &= ~VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		if (stage != 0) return stage;
		if (!source) throw std::invalid_argument("The destination pipeline stage is not supported by this Vulkan command queue.");
		// A semaphore carries the prior queue's memory dependency into this queue.
		access = 0;
		return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}

	void RecordTransferOwnership(const GEngine::Ref<GEngine::CommandBuffer>& commandBuffer,
		const GEngine::Ref<GEngine::GraphicsResource>& resource,
		const GEngine::GraphicsResourceUsage& usage,
		const GEngine::GraphicsSubresourceRange& range, bool release)
	{
		const auto texture = std::dynamic_pointer_cast<GEngine::VulkanTexture2D>(resource);
		const auto array = std::dynamic_pointer_cast<GEngine::VulkanTexture2DArray>(resource);
		if ((!texture || !texture->UsesExclusiveSharing()) && (!array || !array->UsesExclusiveSharing())) return;
		const auto nativeCommand = std::dynamic_pointer_cast<GEngine::VulkanCommandBuffer>(commandBuffer);
		if (!nativeCommand) throw std::invalid_argument("Exclusive Vulkan ownership requires a Vulkan command buffer.");
		const auto type = nativeCommand->GetType();
		const auto families = GEngine::VulkanContext::Get()->GetQueueFamily();
		if (type != GEngine::COMMAND_BUFFER_TYPE_GRAPHICS && type != GEngine::COMMAND_BUFFER_TYPE_TRANSFER)
			throw std::invalid_argument("Exclusive transfer images require graphics or transfer commands.");
		if (families.GraphicsFamily == families.TransferFamily) return;
		if (type == GEngine::COMMAND_BUFFER_TYPE_TRANSFER &&
			usage.State != GEngine::GraphicsResourceState::CopySource &&
			usage.State != GEngine::GraphicsResourceState::CopyDestination)
			throw std::invalid_argument("Transfer queue ownership requires a copy state.");
		const uint32_t mipCount = resource->GetSubresourceMetadata().MipLevelCount;
		const uint32_t layerCount = resource->GetSubresourceMetadata().ArrayLayerCount;
		const uint32_t selectedMips = range.MipLevelCount == GEngine::GraphicsSubresourceRange::All
			? mipCount - range.BaseMipLevel : range.MipLevelCount;
		const uint32_t selectedLayers = range.ArrayLayerCount == GEngine::GraphicsSubresourceRange::All
			? layerCount - range.BaseArrayLayer : range.ArrayLayerCount;
		const bool graphics = type == GEngine::COMMAND_BUFFER_TYPE_GRAPHICS;
		const uint8_t expected = graphics ? (release ? 0 : 3) : (release ? 2 : 1);
		const uint8_t next = graphics ? (release ? 1 : 0) : (release ? 3 : 2);
		const bool graphicsToTransfer = graphics == release;
		const uint32_t sourceFamily = graphicsToTransfer ? families.GraphicsFamily.value() : families.TransferFamily.value();
		const uint32_t destinationFamily = graphicsToTransfer ? families.TransferFamily.value() : families.GraphicsFamily.value();
		const VkPipelineStageFlags sourceStage = release
			? (graphics ? VK_PIPELINE_STAGE_ALL_COMMANDS_BIT : VK_PIPELINE_STAGE_TRANSFER_BIT)
			: VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		const VkPipelineStageFlags destinationStage = release ? VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
			: (graphics ? VK_PIPELINE_STAGE_ALL_COMMANDS_BIT : VK_PIPELINE_STAGE_TRANSFER_BIT);
		const VkAccessFlags transferAccess = usage.State == GEngine::GraphicsResourceState::CopyDestination
			? VK_ACCESS_TRANSFER_WRITE_BIT : VK_ACCESS_TRANSFER_READ_BIT;
		const VkAccessFlags sourceAccess = release
			? (graphics ? VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT : transferAccess) : 0;
		const VkAccessFlags destinationAccess = release ? 0
			: (graphics ? VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT : transferAccess);
		std::vector<VkImageMemoryBarrier> barriers;
		barriers.reserve(static_cast<size_t>(selectedMips) * selectedLayers);
		for (uint32_t layer = range.BaseArrayLayer; layer < range.BaseArrayLayer + selectedLayers; ++layer)
			for (uint32_t mip = range.BaseMipLevel; mip < range.BaseMipLevel + selectedMips; ++mip)
			{
				const uint8_t current = texture ? texture->GetQueueOwnership(mip) : array->GetQueueOwnership(mip, layer);
				if (graphics && !release && current == 0) continue;
				if (current != expected) throw std::invalid_argument("Vulkan image queue ownership is not ready for this handoff.");
				const VkImageLayout layout = texture ? texture->GetSubresourceLayout(mip) : array->GetSubresourceLayout(mip, layer);
				if (layout == VK_IMAGE_LAYOUT_UNDEFINED)
					throw std::invalid_argument("Exclusive transfer image must have a defined layout before handoff.");
				VkImageMemoryBarrier barrier{};
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.srcAccessMask = sourceAccess;
				barrier.dstAccessMask = destinationAccess;
				barrier.oldLayout = layout;
				barrier.newLayout = layout;
				barrier.srcQueueFamilyIndex = sourceFamily;
				barrier.dstQueueFamilyIndex = destinationFamily;
				barrier.image = texture ? texture->GetImage() : array->GetImage();
				barrier.subresourceRange = { texture ? texture->GetAspectFlag() : array->GetAspectFlag(), mip, 1, layer, 1 };
				barriers.push_back(barrier);
				if (texture) texture->SetQueueOwnership(mip, next);
				else array->SetQueueOwnership(mip, layer, next);
			}
		if (!barriers.empty())
			vkCmdPipelineBarrier(nativeCommand->GetCommandBuffer(), sourceStage, destinationStage, 0,
				0, nullptr, 0, nullptr, static_cast<uint32_t>(barriers.size()), barriers.data());
	}

	void RecordComputeOwnership(const GEngine::Ref<GEngine::CommandBuffer>& commandBuffer,
		const GEngine::Ref<GEngine::GraphicsResource>& resource,
		const GEngine::GraphicsResourceUsage& usage,
		const GEngine::GraphicsSubresourceRange& range, bool release)
	{
		const auto buffer = std::dynamic_pointer_cast<GEngine::VulkanStorageBuffer>(resource);
		if (!buffer || !buffer->UsesExclusiveSharing()) return;
		if (!range.IsWholeResource() || usage.State != GEngine::GraphicsResourceState::ShaderWrite)
			throw std::invalid_argument("Exclusive compute buffers require whole-buffer storage access.");
		const auto nativeCommand = std::dynamic_pointer_cast<GEngine::VulkanCommandBuffer>(commandBuffer);
		if (!nativeCommand) throw std::invalid_argument("Exclusive Vulkan ownership requires a Vulkan command buffer.");
		const auto type = nativeCommand->GetType();
		if (type != GEngine::COMMAND_BUFFER_TYPE_GRAPHICS && type != GEngine::COMMAND_BUFFER_TYPE_COMPUTE)
			throw std::invalid_argument("Exclusive compute buffers require graphics or compute commands.");
		const auto families = GEngine::VulkanContext::Get()->GetQueueFamily();
		if (families.GraphicsFamily == families.ComputeFamily) return;
		const bool graphics = type == GEngine::COMMAND_BUFFER_TYPE_GRAPHICS;
		const uint8_t expected = graphics ? (release ? 0 : 3) : (release ? 2 : 1);
		const uint8_t next = graphics ? (release ? 1 : 0) : (release ? 3 : 2);
		const uint8_t current = buffer->GetQueueOwnership();
		if (graphics && !release && current == 0) return;
		if (current != expected)
			throw std::invalid_argument("Vulkan buffer queue ownership is not ready for this handoff.");
		const bool graphicsToCompute = graphics == release;
		VkBufferMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.srcAccessMask = release
			? (graphics ? VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT : VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)
			: 0;
		barrier.dstAccessMask = release ? 0
			: (graphics ? VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT : VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
		barrier.srcQueueFamilyIndex = graphicsToCompute ? families.GraphicsFamily.value() : families.ComputeFamily.value();
		barrier.dstQueueFamilyIndex = graphicsToCompute ? families.ComputeFamily.value() : families.GraphicsFamily.value();
		barrier.buffer = reinterpret_cast<VkBuffer>(buffer->GetNativeResource());
		barrier.offset = 0;
		barrier.size = VK_WHOLE_SIZE;
		const VkPipelineStageFlags sourceStage = release
			? (graphics ? VK_PIPELINE_STAGE_ALL_COMMANDS_BIT : VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT)
			: VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		const VkPipelineStageFlags destinationStage = release ? VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
			: (graphics ? VK_PIPELINE_STAGE_ALL_COMMANDS_BIT : VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		vkCmdPipelineBarrier(nativeCommand->GetCommandBuffer(), sourceStage, destinationStage, 0,
			0, nullptr, 1, &barrier, 0, nullptr);
		buffer->SetQueueOwnership(next);
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
	void VulkanGraphicsAPI::CopyTexture2DToArrayLayer(const Ref<CommandBuffer>& commandBuffer,
		const Ref<Texture2D>& source, const Ref<Texture2DArray>& destination, uint32_t layer)
	{
		auto command = std::dynamic_pointer_cast<VulkanCommandBuffer>(commandBuffer);
		auto nativeSource = std::dynamic_pointer_cast<VulkanTexture2D>(source);
		auto nativeDestination = std::dynamic_pointer_cast<VulkanTexture2DArray>(destination);
		if (!command || (command->GetType() != COMMAND_BUFFER_TYPE_GRAPHICS &&
			command->GetType() != COMMAND_BUFFER_TYPE_TRANSFER) || !nativeSource || !nativeDestination ||
			layer >= destination->GetArrayLayerCount() || source->GetWidth() != destination->GetWidth() ||
			source->GetHeight() != destination->GetHeight() || source->GetFormat() != destination->GetFormat())
			throw std::invalid_argument("Vulkan texture-array copy requires matching textures, a valid layer, and a graphics or transfer command buffer.");
		if (nativeSource->GetSubresourceLayout(0) != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL ||
			nativeDestination->GetSubresourceLayout(0, layer) != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
			throw std::invalid_argument("Vulkan texture-array copy requires declared copy-source and copy-destination states.");
		VkImageCopy region{};
		region.srcSubresource = { nativeSource->GetAspectFlag(), 0, 0, 1 };
		region.dstSubresource = { nativeDestination->GetAspectFlag(), 0, layer, 1 };
		region.extent = { source->GetWidth(), source->GetHeight(), 1 };
		vkCmdCopyImage(command->GetCommandBuffer(), nativeSource->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			nativeDestination->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		command->Retain(source);
		command->Retain(destination);
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
	Ref<CommandBuffer> VulkanGraphicsAPI::GetTransferCommandBuffer()
	{
		return VulkanContext::Get()->GetCommandBuffer(COMMAND_BUFFER_TYPE_TRANSFER);
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
    Ref<Texture2D> VulkanGraphicsAPI::CreateTexture2D(uint32_t width, uint32_t height, RenderImage2DFormat format, uint32_t mipLevelCount)
    {
        return CreateRef<VulkanTexture2D>(width, height, format, mipLevelCount);
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

	void VulkanGraphicsAPI::ReleaseTransferOwnership(const Ref<CommandBuffer>& commandBuffer,
		const Ref<GraphicsResource>& resource, const GraphicsResourceUsage& usage,
		const GraphicsSubresourceRange& range)
	{
		if (!resource) return;
		ValidateSubresourceRange(resource, range);
		RecordTransferOwnership(commandBuffer, resource, usage, range, true);
	}

	void VulkanGraphicsAPI::AcquireTransferOwnership(const Ref<CommandBuffer>& commandBuffer,
		const Ref<GraphicsResource>& resource, const GraphicsResourceUsage& usage,
		const GraphicsSubresourceRange& range)
	{
		if (!resource) return;
		ValidateSubresourceRange(resource, range);
		RecordTransferOwnership(commandBuffer, resource, usage, range, false);
	}

	void VulkanGraphicsAPI::ReleaseComputeOwnership(const Ref<CommandBuffer>& commandBuffer,
		const Ref<GraphicsResource>& resource, const GraphicsResourceUsage& usage,
		const GraphicsSubresourceRange& range)
	{
		if (!resource) return;
		ValidateSubresourceRange(resource, range);
		RecordComputeOwnership(commandBuffer, resource, usage, range, true);
	}

	void VulkanGraphicsAPI::AcquireComputeOwnership(const Ref<CommandBuffer>& commandBuffer,
		const Ref<GraphicsResource>& resource, const GraphicsResourceUsage& usage,
		const GraphicsSubresourceRange& range)
	{
		if (!resource) return;
		ValidateSubresourceRange(resource, range);
		RecordComputeOwnership(commandBuffer, resource, usage, range, false);
	}

	RenderDevice::QueueOwnershipKind VulkanGraphicsAPI::GetQueueOwnershipKind(const Ref<GraphicsResource>& resource) const
	{
		const auto families = VulkanContext::Get()->GetQueueFamily();
		if (const auto buffer = std::dynamic_pointer_cast<VulkanStorageBuffer>(resource);
			buffer && buffer->UsesExclusiveSharing() &&
			families.GraphicsFamily != families.ComputeFamily)
			return QueueOwnershipKind::ComputeBuffer;
		if (const auto texture = std::dynamic_pointer_cast<VulkanTexture2D>(resource);
			texture && texture->UsesExclusiveSharing()) return QueueOwnershipKind::TransferImage;
		if (const auto array = std::dynamic_pointer_cast<VulkanTexture2DArray>(resource);
			array && array->UsesExclusiveSharing()) return QueueOwnershipKind::TransferImage;
		return QueueOwnershipKind::None;
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
		TransitionResource(commandBuffer, resource, { before }, { after });
	}

	void VulkanGraphicsAPI::TransitionResource(const Ref<CommandBuffer>& commandBuffer, const Ref<GraphicsResource>& resource,
		const GraphicsResourceUsage& before, const GraphicsResourceUsage& after)
	{
		const auto nativeResource = GetNativeResource(resource);
		if (!resource || nativeResource == nullptr)
			return;

		auto vulkanCommandBuffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(commandBuffer);
		GE_CORE_ASSERT(vulkanCommandBuffer, "Vulkan resource transitions require a Vulkan command buffer.");
		if (before.State == after.State)
		{
			if (before.State == GraphicsResourceState::ShaderWrite)
			{
				VkMemoryBarrier barrier{};
				barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
				barrier.srcAccessMask = ToVulkanAccess(before.Access, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
				barrier.dstAccessMask = ToVulkanAccess(after.Access, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
				const auto sourceStage = StageForCommandBuffer(
					ToVulkanStage(before.Stage, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT), barrier.srcAccessMask,
					vulkanCommandBuffer->GetType(), true);
				const auto destinationStage = StageForCommandBuffer(
					ToVulkanStage(after.Stage, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT), barrier.dstAccessMask,
					vulkanCommandBuffer->GetType(), false);
				vkCmdPipelineBarrier(vulkanCommandBuffer->GetCommandBuffer(),
					sourceStage, destinationStage, 0, 1, &barrier, 0, nullptr, 0, nullptr);
			}
			return;
		}
		auto source = ToVulkanResourceState(before.State);
		auto destination = ToVulkanResourceState(after.State);
		source.Stage = ToVulkanStage(before.Stage, source.Stage);
		destination.Stage = ToVulkanStage(after.Stage, destination.Stage);
		source.Access = ToVulkanAccess(before.Access, source.Access);
		destination.Access = ToVulkanAccess(after.Access, destination.Access);
		source.Stage = StageForCommandBuffer(source.Stage, source.Access, vulkanCommandBuffer->GetType(), true);
		destination.Stage = StageForCommandBuffer(destination.Stage, destination.Access, vulkanCommandBuffer->GetType(), false);
		if (auto texture = std::dynamic_pointer_cast<VulkanTexture2D>(resource))
		{
			texture->SetImageLayout(vulkanCommandBuffer->GetCommandBuffer(), destination.Layout);
			return;
		}
		if (auto textureArray = std::dynamic_pointer_cast<VulkanTexture2DArray>(resource))
		{
			textureArray->SetImageLayout(vulkanCommandBuffer->GetCommandBuffer(), destination.Layout);
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
		barrier.subresourceRange.aspectMask = (before.State == GraphicsResourceState::DepthWrite || after.State == GraphicsResourceState::DepthWrite)
			? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
		vkCmdPipelineBarrier(vulkanCommandBuffer->GetCommandBuffer(), source.Stage, destination.Stage, 0,
			0, nullptr, 0, nullptr, 1, &barrier);
	}

	void VulkanGraphicsAPI::TransitionResource(const Ref<CommandBuffer>& commandBuffer,
		const Ref<GraphicsResource>& resource, const GraphicsResourceUsage& before,
		const GraphicsResourceUsage& after, const GraphicsSubresourceRange& range)
	{
		if (!resource) return;
		ValidateSubresourceRange(resource, range);
		const auto metadata = resource->GetSubresourceMetadata();
		const bool hasMetadata = metadata.MipLevelCount != 0 && metadata.ArrayLayerCount != 0;
		const uint32_t mipCount = hasMetadata ? metadata.MipLevelCount : 1;
		const uint32_t layerCount = hasMetadata ? metadata.ArrayLayerCount : 1;
		const uint32_t selectedMipCount = range.MipLevelCount == GraphicsSubresourceRange::All
			? mipCount - std::min(range.BaseMipLevel, mipCount) : range.MipLevelCount;
		const uint32_t selectedLayerCount = range.ArrayLayerCount == GraphicsSubresourceRange::All
			? layerCount - std::min(range.BaseArrayLayer, layerCount) : range.ArrayLayerCount;
		if (selectedMipCount == 0 || selectedLayerCount == 0 || range.BaseMipLevel >= mipCount ||
			range.BaseArrayLayer >= layerCount || selectedMipCount > mipCount - range.BaseMipLevel ||
			selectedLayerCount > layerCount - range.BaseArrayLayer)
			throw std::invalid_argument("Vulkan subresource range is empty or out of bounds.");
		const bool wholeRange = hasMetadata ? range.IsWholeResource(mipCount, layerCount) : range.IsWholeResource();

		auto vulkanCommandBuffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(commandBuffer);
		if (!vulkanCommandBuffer) throw std::invalid_argument("Vulkan resource transitions require a Vulkan command buffer.");
		if (resource->GetResourceType() == GraphicsResourceType::Buffer)
		{
			if (!wholeRange) throw std::invalid_argument("Vulkan buffer transitions do not support subresource ranges.");
			TransitionResource(commandBuffer, resource, before, after);
			return;
		}

		auto texture = std::dynamic_pointer_cast<VulkanTexture2D>(resource);
		auto textureArray = std::dynamic_pointer_cast<VulkanTexture2DArray>(resource);
		auto storageImage = std::dynamic_pointer_cast<VulkanStorageImage2D>(resource);
		if (!texture && !textureArray && !storageImage)
		{
			if (!wholeRange) throw std::invalid_argument("This Vulkan image type does not support subresource ranges.");
			TransitionResource(commandBuffer, resource, before, after);
			return;
		}

		const auto nativeCommandBuffer = vulkanCommandBuffer->GetCommandBuffer();
		if (before.State == after.State)
		{
			if (before.State != GraphicsResourceState::ShaderWrite) return;
			VkMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			barrier.srcAccessMask = ToVulkanAccess(before.Access, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
			barrier.dstAccessMask = ToVulkanAccess(after.Access, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
			const auto sourceStage = StageForCommandBuffer(
				ToVulkanStage(before.Stage, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT), barrier.srcAccessMask,
				vulkanCommandBuffer->GetType(), true);
			const auto destinationStage = StageForCommandBuffer(
				ToVulkanStage(after.Stage, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT), barrier.dstAccessMask,
				vulkanCommandBuffer->GetType(), false);
			vkCmdPipelineBarrier(nativeCommandBuffer, sourceStage,
				destinationStage, 0, 1, &barrier, 0, nullptr, 0, nullptr);
			return;
		}

		auto source = ToVulkanResourceState(before.State);
		auto destination = ToVulkanResourceState(after.State);
		source.Stage = ToVulkanStage(before.Stage, source.Stage);
		destination.Stage = ToVulkanStage(after.Stage, destination.Stage);
		source.Access = ToVulkanAccess(before.Access, source.Access);
		destination.Access = ToVulkanAccess(after.Access, destination.Access);
		source.Stage = StageForCommandBuffer(source.Stage, source.Access, vulkanCommandBuffer->GetType(), true);
		destination.Stage = StageForCommandBuffer(destination.Stage, destination.Access, vulkanCommandBuffer->GetType(), false);
		const VkImage image = reinterpret_cast<VkImage>(GetNativeResource(resource));
		const VkImageAspectFlags aspect = texture ? texture->GetAspectFlag() :
			textureArray ? textureArray->GetAspectFlag() : storageImage->GetImageLayout() == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		std::vector<VkImageMemoryBarrier> barriers;
		barriers.reserve(static_cast<size_t>(selectedMipCount) * selectedLayerCount);
		for (uint32_t layer = range.BaseArrayLayer; layer < range.BaseArrayLayer + selectedLayerCount; ++layer)
			for (uint32_t mip = range.BaseMipLevel; mip < range.BaseMipLevel + selectedMipCount; ++mip)
			{
				const VkImageLayout oldLayout = texture ? texture->GetSubresourceLayout(mip) :
					textureArray ? textureArray->GetSubresourceLayout(mip, layer) : storageImage->GetImageLayout();
				VkImageMemoryBarrier barrier{};
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.srcAccessMask = oldLayout == VK_IMAGE_LAYOUT_UNDEFINED ? 0 : source.Access;
				barrier.dstAccessMask = destination.Access;
				barrier.oldLayout = oldLayout;
				barrier.newLayout = destination.Layout;
				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.image = image;
				barrier.subresourceRange = { aspect, mip, 1, layer, 1 };
				barriers.push_back(barrier);
			}
		const VkPipelineStageFlags sourceStage = before.State == GraphicsResourceState::Undefined
			? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : source.Stage;
		vkCmdPipelineBarrier(nativeCommandBuffer, sourceStage, destination.Stage, 0,
			0, nullptr, 0, nullptr, static_cast<uint32_t>(barriers.size()), barriers.data());
		for (uint32_t layer = range.BaseArrayLayer; layer < range.BaseArrayLayer + selectedLayerCount; ++layer)
			for (uint32_t mip = range.BaseMipLevel; mip < range.BaseMipLevel + selectedMipCount; ++mip)
			{
				if (texture) texture->SetSubresourceLayout(mip, destination.Layout);
				else if (textureArray) textureArray->SetSubresourceLayout(mip, layer, destination.Layout);
				else storageImage->SetTrackedImageLayout(destination.Layout);
			}
	}

	void VulkanGraphicsAPI::ValidateSubresourceRange(const Ref<GraphicsResource>& resource,
		const GraphicsSubresourceRange& range) const
	{
		if (!resource) throw std::invalid_argument("Vulkan subresource validation requires a resource.");
		const auto metadata = resource->GetSubresourceMetadata();
		if (metadata.MipLevelCount == 0 || metadata.ArrayLayerCount == 0)
		{
			if (!range.IsWholeResource()) throw std::invalid_argument("Vulkan buffers do not support subresource ranges.");
			return;
		}
		const auto mipCount = metadata.MipLevelCount;
		const auto layerCount = metadata.ArrayLayerCount;
		const auto selectedMips = range.MipLevelCount == GraphicsSubresourceRange::All
			? (range.BaseMipLevel < mipCount ? mipCount - range.BaseMipLevel : 0) : range.MipLevelCount;
		const auto selectedLayers = range.ArrayLayerCount == GraphicsSubresourceRange::All
			? (range.BaseArrayLayer < layerCount ? layerCount - range.BaseArrayLayer : 0) : range.ArrayLayerCount;
		if (range.BaseMipLevel >= mipCount || range.BaseArrayLayer >= layerCount || selectedMips == 0 || selectedLayers == 0 ||
			selectedMips > mipCount - range.BaseMipLevel || selectedLayers > layerCount - range.BaseArrayLayer)
			throw std::invalid_argument("Vulkan subresource range is empty or out of bounds.");
		if (range.IsWholeResource(mipCount, layerCount)) return;
		if (!std::dynamic_pointer_cast<VulkanTexture2D>(resource) &&
			!std::dynamic_pointer_cast<VulkanTexture2DArray>(resource))
			throw std::invalid_argument("This Vulkan image type does not support subresource ranges.");
	}

}
