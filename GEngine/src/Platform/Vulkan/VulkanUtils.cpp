#include "GEpch.h"
#include "VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace GEngine
{
    namespace Utils
    {
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDvice)
        {
            VkPhysicalDeviceMemoryProperties    memProperties;
            vkGetPhysicalDeviceMemoryProperties(physicalDvice, &memProperties);
            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }

            GE_CORE_ASSERT(false, "failed to find suitable memory type!");
            return 0;
        }

        void CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) 
        {

            VkBufferCreateInfo      bufferInfo{};
            bufferInfo.sType        = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size         = size;
            bufferInfo.usage        = usage;
            bufferInfo.sharingMode  = VK_SHARING_MODE_EXCLUSIVE;

			VK_CHECK_RESULT(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer));

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

            VkMemoryAllocateInfo        allocInfo{};
            allocInfo.sType             = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize    = memRequirements.size;
            allocInfo.memoryTypeIndex   = FindMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice);

            VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory));
            vkBindBufferMemory(device, buffer, bufferMemory, 0);
        }

		void CreateImages(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t width, uint32_t height, VkFormat format, 
			VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkSampleCountFlagBits sample, 
			uint32_t arrayLayers, VkImageCreateFlags flags, uint32_t mipLevel, VkImage& outImage, VkDeviceMemory& imageMemory)
		{
			VkImageCreateInfo		imageInfo{};
			imageInfo.sType			= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType		= VK_IMAGE_TYPE_2D;
			imageInfo.extent.width	= width;
			imageInfo.extent.height = height;
			imageInfo.extent.depth	= 1;
			imageInfo.mipLevels		= mipLevel;
			imageInfo.arrayLayers	= arrayLayers;
			imageInfo.format		= format;
			imageInfo.tiling		= tiling;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage			= usage;
			imageInfo.samples		= sample;
			imageInfo.sharingMode	= VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.flags			= flags; // Optional

            VK_CHECK_RESULT(vkCreateImage(device, &imageInfo, nullptr, &outImage));

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(device, outImage, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice);

			VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory));

			vkBindImageMemory(device, outImage, imageMemory, 0);
		}

		void CreateImageViews(VkDevice device, VkImage image, VkFormat format, VkImageViewType viewType, uint32_t layerCount, VkImageAspectFlags aspectMask, uint32_t mipLevel, VkImageView& outImageView)
		{
			VkImageViewCreateInfo                           createInfo{};
			createInfo.sType								= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image								= image;
			createInfo.viewType								= viewType;
			createInfo.format								= format;
			createInfo.components.r							= VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g							= VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b							= VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a							= VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask			= aspectMask;
			createInfo.subresourceRange.baseMipLevel		= 0;
			createInfo.subresourceRange.levelCount			= mipLevel;
			createInfo.subresourceRange.baseArrayLayer		= 0;
			createInfo.subresourceRange.layerCount			= layerCount;

			VK_CHECK_RESULT(vkCreateImageView(device, &createInfo, nullptr, &outImageView));
		}
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layerCount, VkFlags aspectFlag, uint32_t mipLevel)
		{
			VkCommandBuffer	commandBuffer = VulkanContext::Get()->BeginSingleTimeCommands();

			VkImageMemoryBarrier		barrier{};
			barrier.sType				= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout			= oldLayout;
			barrier.newLayout			= newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image				= image;
			barrier.subresourceRange.aspectMask		= aspectFlag;
			barrier.subresourceRange.baseMipLevel	= 0;
			barrier.subresourceRange.levelCount		= mipLevel;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount		= layerCount;
			barrier.srcAccessMask					= 0; // TODO
			barrier.dstAccessMask					= 0; // TODO

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;


			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage			= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage	= VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;

				sourceStage			= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
				destinationStage	= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage			= VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage	= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

				sourceStage			= VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage	= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL)
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

				sourceStage			= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage	= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				destinationStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			{
				barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

				sourceStage			= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
				destinationStage	= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage			= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				destinationStage	= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

				sourceStage			= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
				destinationStage	= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			}
			else {
				VulkanContext::Get()->EndSingleTimeCommands(commandBuffer);
				return;
			}

			vkCmdPipelineBarrier(
				commandBuffer,
				sourceStage, destinationStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			VulkanContext::Get()->EndSingleTimeCommands(commandBuffer);
		}
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t mipLevel, uint32_t baseArrayLayer, VkFlags aspectFlag)
		{
			VkCommandBuffer	commandBuffer = VulkanContext::Get()->BeginSingleTimeCommands();

			VkBufferImageCopy					region{};
			region.bufferOffset					= 0;
			region.bufferRowLength				= 0;
			region.bufferImageHeight			= 0;

			region.imageSubresource.aspectMask	= aspectFlag;
			region.imageSubresource.mipLevel	= mipLevel;
			region.imageSubresource.baseArrayLayer = baseArrayLayer;
			region.imageSubresource.layerCount	= 1;

			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = {
				width,
				height,
				1
			};

			vkCmdCopyBufferToImage(
				commandBuffer,
				buffer,
				image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&region
			);

			VulkanContext::Get()->EndSingleTimeCommands(commandBuffer);
		}


		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
		{
			for (VkFormat format : candidates) {
				VkFormatProperties props;
				vkGetPhysicalDeviceFormatProperties(VulkanContext::Get()->GetPhysicalDevice(), format, &props);

				if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
					return format;
				}
				else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
					return format;
				}
			}

			GE_CORE_CRITICAL("failed to find supported format!");
		}

		VkAttachmentDescription2 CreateAttachmentDescription2(FrameBufferTextureFormat format, VkSampleCountFlagBits sample)
		{
			VkAttachmentDescription2	Attachment{};
			Attachment.sType			= VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
			Attachment.samples			= sample;
			Attachment.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
			Attachment.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
			Attachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			Attachment.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			Attachment.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
			switch (format)
			{
			case GEngine::FrameBufferTextureFormat::None:
				GE_CORE_ASSERT(false, "None is not a valid FrameBufferTextureFormat!");
				break;
			case GEngine::FrameBufferTextureFormat::RGBA8:
				Attachment.format		= VK_FORMAT_R8G8B8A8_UNORM;
				Attachment.finalLayout	= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				break;
			case GEngine::FrameBufferTextureFormat::RED_INTEGER:
				Attachment.format		= VK_FORMAT_R8_SINT;
				Attachment.finalLayout	= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				break;
			case GEngine::FrameBufferTextureFormat::DEPTH24STENCIL8:
				Attachment.format			= FindSupportedFormat({ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
				Attachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_CLEAR;
				Attachment.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_STORE;
				Attachment.finalLayout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				break;
			case GEngine::FrameBufferTextureFormat::DEPTH:
				Attachment.format		= FindSupportedFormat({ VK_FORMAT_D32_SFLOAT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
				Attachment.finalLayout	= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				break;
			default:
				break;
			}

			return Attachment;
		}

		VkAttachmentReference2 CreateAttachmentReference2(FrameBufferTextureFormat format, int index)
		{
			VkAttachmentReference2		Ref{};
			Ref.attachment = index;
			Ref.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
			switch (format)
			{
			case GEngine::FrameBufferTextureFormat::None:
				GE_CORE_ASSERT(false, "Unknown framebuffer texture format");
				break;
			case GEngine::FrameBufferTextureFormat::RGBA8:
				Ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				Ref.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				break;
			case GEngine::FrameBufferTextureFormat::RED_INTEGER:
				Ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				Ref.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				break;
			case GEngine::FrameBufferTextureFormat::DEPTH24STENCIL8:
				Ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				Ref.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
				break;
			case GEngine::FrameBufferTextureFormat::DEPTH:
				Ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				Ref.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				break;
			default:
				break;
			}

			return Ref;
		}
		VkAttachmentDescription CreateAttachmentDescription(FrameBufferTextureFormat format, VkSampleCountFlagBits sample)
		{
			VkAttachmentDescription	Attachment{};
			Attachment.samples = sample;
			Attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			Attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			Attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			Attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			Attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			switch (format)
			{
			case GEngine::FrameBufferTextureFormat::None:
				GE_CORE_ASSERT(false, "None is not a valid FrameBufferTextureFormat!");
				break;
			case GEngine::FrameBufferTextureFormat::RGBA8:
				Attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
				Attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				break;
			case GEngine::FrameBufferTextureFormat::RED_INTEGER:
				Attachment.format = VK_FORMAT_R8_SINT;
				Attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				break;
			case GEngine::FrameBufferTextureFormat::DEPTH24STENCIL8:
				Attachment.format = FindSupportedFormat({ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
				Attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				Attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
				Attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				break;
			case GEngine::FrameBufferTextureFormat::DEPTH:
				Attachment.format = FindSupportedFormat({ VK_FORMAT_D32_SFLOAT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
				Attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				break;
			default:
				break;
			}

			return Attachment;
		}
		VkAttachmentReference CreateAttachmentReference(FrameBufferTextureFormat format, int index)
		{
			VkAttachmentReference		Ref{};
			Ref.attachment = index;
			switch (format)
			{
			case GEngine::FrameBufferTextureFormat::None:
				GE_CORE_ASSERT(false, "Unknown framebuffer texture format");
				break;
			case GEngine::FrameBufferTextureFormat::RGBA8:
				Ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				break;
			case GEngine::FrameBufferTextureFormat::RED_INTEGER:
				Ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				break;
			case GEngine::FrameBufferTextureFormat::DEPTH24STENCIL8:
				Ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				break;
			case GEngine::FrameBufferTextureFormat::DEPTH:
				Ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				break;
			default:
				break;
			}

			return Ref;
		}
		VkSampleCountFlagBits SampleCountToVulkanFlag(int sample)
		{
			switch (sample)
			{
			case 1: return VK_SAMPLE_COUNT_1_BIT;
			case 2: return VK_SAMPLE_COUNT_2_BIT;
			case 4: return VK_SAMPLE_COUNT_4_BIT;
			case 8: return VK_SAMPLE_COUNT_8_BIT;
			case 16: return VK_SAMPLE_COUNT_16_BIT;
			case 32: return VK_SAMPLE_COUNT_32_BIT;
			case 64: return VK_SAMPLE_COUNT_64_BIT;
			default:
				GE_CORE_ASSERT(false, "invalied sample value");
				break;
			}
			return VK_SAMPLE_COUNT_1_BIT;
		}
		void BlitImage(VkImage src, VkImageLayout srcLayout, Vector2 srcSize, uint32_t srcMipLevel, VkImage dst, VkImageLayout dstLayout, Vector2 dstSize, uint32_t dstMipLevel, VkImageAspectFlags aspectFlag)
		{
			VkCommandBuffer	commandBuffer = VulkanContext::Get()->BeginSingleTimeCommands();

			VkImageBlit		region{};
			region.srcSubresource.mipLevel = srcMipLevel;
			region.srcSubresource.aspectMask = aspectFlag;
			region.srcSubresource.baseArrayLayer = 0;
			region.srcSubresource.layerCount = 1;
			region.srcOffsets[0] = { 0, 0, 0 };
			region.srcOffsets[1] = { (int)srcSize.x, (int)srcSize.y, 1 };
			region.dstSubresource.mipLevel = dstMipLevel;
			region.dstSubresource.aspectMask = aspectFlag;
			region.dstSubresource.baseArrayLayer = 0;
			region.dstSubresource.layerCount = 1;
			region.dstOffsets[0] = { 0, 0, 0 };
			region.dstOffsets[1] = { (int)dstSize.x, (int)dstSize.y, 1 };

			vkCmdBlitImage(commandBuffer,
				src,
				srcLayout,
				dst,
				dstLayout,
				1,
				&region,
				VK_FILTER_LINEAR);

			VulkanContext::Get()->EndSingleTimeCommands(commandBuffer);
		}
		void BlitImage(VkCommandBuffer CmdBuffer, VkImage src, VkImageLayout srcLayout, Vector2 srcSize, uint32_t srcMipLevel, VkImage dst, VkImageLayout dstLayout, Vector2 dstSize, uint32_t dstMipLevel, VkImageAspectFlags aspectFlag)
		{
			VkImageBlit								region{};
			region.srcSubresource.mipLevel			= srcMipLevel;
			region.srcSubresource.aspectMask		= aspectFlag;
			region.srcSubresource.baseArrayLayer	= 0;
			region.srcSubresource.layerCount		= 1;
			region.srcOffsets[0]					= { 0, 0, 0 };
			region.srcOffsets[1]					= { (int)srcSize.x, (int)srcSize.y, 1 };
			region.dstSubresource.mipLevel			= dstMipLevel;
			region.dstSubresource.aspectMask		= aspectFlag;
			region.dstSubresource.baseArrayLayer	= 0;
			region.dstSubresource.layerCount		= 1;
			region.dstOffsets[0]					= { 0, 0, 0 };
			region.dstOffsets[1]					= { (int)dstSize.x, (int)dstSize.y, 1 };

			vkCmdBlitImage(CmdBuffer,
				src,
				srcLayout,
				dst,
				dstLayout,
				1,
				&region,
				VK_FILTER_LINEAR);
		}
		void CopyBufferToBuffer(VkBuffer src, VkBuffer dst, uint32_t size)
		{
			VkCommandBuffer	commandBuffer = VulkanContext::Get()->BeginSingleTimeCommands();

			VkBufferCopy	copyRegion{};
			copyRegion.srcOffset = 0;
			copyRegion.dstOffset = 0;
			copyRegion.size = size;

			vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);

			VulkanContext::Get()->EndSingleTimeCommands(commandBuffer);
		}

		VkBlendFactor BlendFactorToVulkanBlendFactor(BlendFactor factor)
		{
			switch (factor)
			{
			case BlendFactor::SRC_ALPHA: return VK_BLEND_FACTOR_SRC_ALPHA;
			case BlendFactor::DST_ALPHA: return VK_BLEND_FACTOR_DST_ALPHA;
			case BlendFactor::SRC_COLOR: return VK_BLEND_FACTOR_SRC_COLOR;
			case BlendFactor::DST_COLOR: return VK_BLEND_FACTOR_DST_COLOR;
			case BlendFactor::ONE_MINUS_SRC_ALPHA: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			case BlendFactor::ONE_MINUS_DST_ALPHA: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			case BlendFactor::ONE_MINUS_SRC_COLOR: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
			case BlendFactor::ONE_MINUS_DST_COLOR: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
			case BlendFactor::ONE: return VK_BLEND_FACTOR_ONE;
			case BlendFactor::ZERO: return VK_BLEND_FACTOR_ZERO;
			default:
				break;
			}
			return VkBlendFactor();
		}

		VkFormat RenderImage2DFormatToVulkanFormat(RenderImage2DFormat format)
		{
			switch (format)
			{
			case RenderImage2DFormat::RGBA8F: return VK_FORMAT_R8G8B8A8_UNORM;
			case RenderImage2DFormat::RGB8F:   return VK_FORMAT_R8G8B8_UNORM;
				break;
			default:
				break;
			}
		}
		VkFormat ComputeImage2DFormatToVulkanFormat(ComputeImage2DFormat format)
		{
			switch (format)
			{
			case ComputeImage2DFormat::RGBA32F: return VK_FORMAT_R32G32B32A32_SFLOAT;
			case ComputeImage2DFormat::RGBA16F: return VK_FORMAT_R16G16B16A16_SFLOAT;
			case ComputeImage2DFormat::RG32F: return VK_FORMAT_R32G32_SFLOAT;
			case ComputeImage2DFormat::RG16F: return VK_FORMAT_R16G16_SFLOAT;
			case ComputeImage2DFormat::R32F: return VK_FORMAT_R32_SFLOAT;
			case ComputeImage2DFormat::R16F: return VK_FORMAT_R16_SFLOAT;
			case ComputeImage2DFormat::RGBA32I: return VK_FORMAT_R32G32B32A32_SINT;
			case ComputeImage2DFormat::RGBA16I: return VK_FORMAT_R16G16B16A16_SINT;
			case ComputeImage2DFormat::RGBA8I: return VK_FORMAT_R8G8B8A8_SINT;
			case ComputeImage2DFormat::RGBA32UI: return VK_FORMAT_R32G32B32A32_UINT;
			case ComputeImage2DFormat::RGBA16UI: return VK_FORMAT_R16G16B16A16_UINT;
			case ComputeImage2DFormat::RGBA8UI: return VK_FORMAT_R8G8B8A8_UINT;
			case ComputeImage2DFormat::R32UI: return VK_FORMAT_R32_UINT;
			default:
				break;
			}
			return VkFormat();
		}
		RenderImage2DFormat VulkanFormatToRenderImage2DFormat(VkFormat format)
		{
			switch (format)
			{
			case VK_FORMAT_R8G8B8A8_UNORM:			return RenderImage2DFormat::RGBA8F;
			case VK_FORMAT_R8G8B8_UNORM:			return RenderImage2DFormat::RGB8F;
				break;
			default:
				break;
			}
		}
		// front和back要反过来，适应viewport的反转
		VkCullModeFlagBits CullModeToVkCullMode(CullMode mode)
		{
			switch (mode)
			{
			case CullMode::None:	return VK_CULL_MODE_NONE;
			case CullMode::Front:	return VK_CULL_MODE_BACK_BIT;
			case CullMode::Back:	return VK_CULL_MODE_FRONT_BIT;
			default:
				break;
			}
		}
		void CopyImageToImage(uint32_t width, uint32_t height, VkImage src, VkImageLayout srcLayout, VkImageAspectFlags srcAspectFlag, uint32_t srcMipLevel, uint32_t srcBaseArrayLayer, VkImage dst, VkImageLayout dstLayout, VkImageAspectFlags dstAspectFlag, uint32_t dstMipLevel, uint32_t dstBaseArrayLayer)
		{
			VkCommandBuffer	commandBuffer = VulkanContext::Get()->BeginSingleTimeCommands();
			VkImageSubresourceLayers	srcLayers{};
			srcLayers.aspectMask			= srcAspectFlag;
			srcLayers.mipLevel				= srcMipLevel;
			srcLayers.baseArrayLayer		= srcBaseArrayLayer;
			srcLayers.layerCount			= 1;

			VkImageSubresourceLayers	dstLayers{};
			dstLayers.aspectMask		= dstAspectFlag;
			dstLayers.mipLevel			= dstMipLevel;
			dstLayers.baseArrayLayer	= dstBaseArrayLayer;
			dstLayers.layerCount		= 1;

			VkImageCopy				region{};
			region.srcSubresource	= srcLayers;
			region.srcOffset		= { 0, 0, 0 };
			region.dstSubresource	= dstLayers;
			region.dstOffset		= { 0, 0, 0 };
			region.extent			= { width, height, 1 };

			vkCmdCopyImage(commandBuffer,
				src,
				srcLayout,
				dst,
				dstLayout,
				1,
				&region);

			VulkanContext::Get()->EndSingleTimeCommands(commandBuffer);
		}
		void GenerateMipmap(VkImage image, uint32_t width, uint32_t height, uint32_t mipLevel, VkImageAspectFlags aspectFlag, uint32_t baseArrayLayer, uint32_t layerCount)
		{
			VkCommandBuffer	commandBuffer = VulkanContext::Get()->BeginSingleTimeCommands();

			VkImageMemoryBarrier	barrier{};
			barrier.sType				= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.image				= image;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.subresourceRange.aspectMask			= aspectFlag;
			barrier.subresourceRange.baseArrayLayer		= baseArrayLayer;
			barrier.subresourceRange.layerCount			= layerCount;
			barrier.subresourceRange.levelCount			= 1;

			int32_t mipWidth = width;
			int32_t mipHeight = height;

			for (uint32_t i = 1; i < mipLevel; i++)
			{
				barrier.subresourceRange.baseMipLevel	= i - 1;
				barrier.oldLayout						= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout						= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcAccessMask					= VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask					= VK_ACCESS_TRANSFER_READ_BIT;

				vkCmdPipelineBarrier(commandBuffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
					0, nullptr,
					0, nullptr,
					1, &barrier);

				BlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, { mipWidth, mipHeight }, i - 1,
					image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1 }, i,
					aspectFlag);

				/*VkImageBlit		region{};
				region.srcSubresource.aspectMask = aspectFlag;
				region.srcSubresource.baseArrayLayer = baseArrayLayer;
				region.srcSubresource.layerCount = layerCount;
				region.srcSubresource.mipLevel = i - 1;
				region.srcOffsets[0] = { 0, 0, 0 };
				region.srcOffsets[1] = { mipWidth, mipHeight, 1 };
				region.dstSubresource.mipLevel = i;
				region.dstSubresource.aspectMask = aspectFlag;
				region.dstSubresource.baseArrayLayer = baseArrayLayer;
				region.dstSubresource.layerCount = layerCount;
				region.dstOffsets[0] = { 0, 0, 0 };
				region.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };

				vkCmdBlitImage(commandBuffer,
					image,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					image,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&region,
					VK_FILTER_LINEAR);*/
				
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(commandBuffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0,
					0, nullptr,
					0, nullptr,
					1, &barrier);

				if (mipWidth > 1) mipWidth /= 2;
				if (mipHeight > 1) mipHeight /= 2;
			}

			barrier.subresourceRange.baseMipLevel = mipLevel - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			VulkanContext::Get()->EndSingleTimeCommands(commandBuffer);
		}
		VkCompareOp CompareOPToVkCompareOP(CompareOperation op)
		{
			switch (op)
			{
			case CompareOperation::Less:		return VK_COMPARE_OP_LESS;
			case CompareOperation::Greater:		return VK_COMPARE_OP_GREATER;
			case CompareOperation::LessEqual:	return VK_COMPARE_OP_LESS_OR_EQUAL;
			case CompareOperation::GreaterEqual:return VK_COMPARE_OP_GREATER_OR_EQUAL;
			case CompareOperation::Equal:		return VK_COMPARE_OP_EQUAL;
			case CompareOperation::NotEqual:	return VK_COMPARE_OP_NOT_EQUAL;
			case CompareOperation::Always:		return VK_COMPARE_OP_ALWAYS;
			default:
				break;
			}
			return VK_COMPARE_OP_LESS_OR_EQUAL;
		}
    }
}