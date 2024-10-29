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

		void CreateImages(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& outImage, VkDeviceMemory& imageMemory)
		{
			VkImageCreateInfo		imageInfo{};
			imageInfo.sType			= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType		= VK_IMAGE_TYPE_2D;
			imageInfo.extent.width	= width;
			imageInfo.extent.height = height;
			imageInfo.extent.depth	= 1;
			imageInfo.mipLevels		= 1;
			imageInfo.arrayLayers	= 1;
			imageInfo.format		= format;
			imageInfo.tiling		= tiling;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage			= usage;
			imageInfo.samples		= VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode	= VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.flags			= 0; // Optional

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

		void CreateImageViews(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectMask, VkImageView& outImageView)
		{
			VkImageViewCreateInfo                           createInfo{};
			createInfo.sType								= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image								= image;
			createInfo.viewType								= VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format								= format;
			createInfo.components.r							= VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g							= VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b							= VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a							= VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask			= aspectMask;
			createInfo.subresourceRange.baseMipLevel		= 0;
			createInfo.subresourceRange.levelCount			= 1;
			createInfo.subresourceRange.baseArrayLayer		= 0;
			createInfo.subresourceRange.layerCount			= 1;

			VK_CHECK_RESULT(vkCreateImageView(device, &createInfo, nullptr, &outImageView));
		}
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkFlags aspectFlag)
		{
			VkCommandBuffer	commandBuffer = VulkanContext::BeginSingleTimeCommands();

			VkImageMemoryBarrier		barrier{};
			barrier.sType				= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout			= oldLayout;
			barrier.newLayout			= newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image				= image;
			barrier.subresourceRange.aspectMask		= aspectFlag;
			barrier.subresourceRange.baseMipLevel	= 0;
			barrier.subresourceRange.levelCount		= 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount		= 1;
			barrier.srcAccessMask					= 0; // TODO
			barrier.dstAccessMask					= 0; // TODO

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;


			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
			}
			else {
				GE_CORE_ASSERT(false, "unsupported layout transition!");
			}

			vkCmdPipelineBarrier(
				commandBuffer,
				sourceStage, destinationStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			VulkanContext::EndSingleTimeCommands(commandBuffer);
		}
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkFlags aspectFlag)
		{
			VkCommandBuffer	commandBuffer = VulkanContext::BeginSingleTimeCommands();

			VkBufferImageCopy					region{};
			region.bufferOffset					= 0;
			region.bufferRowLength				= 0;
			region.bufferImageHeight			= 0;

			region.imageSubresource.aspectMask	= aspectFlag;
			region.imageSubresource.mipLevel	= 0;
			region.imageSubresource.baseArrayLayer = 0;
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

			VulkanContext::EndSingleTimeCommands(commandBuffer);
		}
    }
}