#include "GEpch.h"
#include "VulkanUtils.h"

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

            GE_CORE_ERROR("failed to find suitable memory type!");
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

		void CreateImages(VkDevice device, uint32_t width, uint32_t height, VkFormat format, int samples, VkImage& outImage)
		{
			VkImageCreateInfo		imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = width;
			imageInfo.extent.height = height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = format;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.flags = 0; // Optional

            VK_CHECK_RESULT(vkCreateImage(device, &imageInfo, nullptr, &outImage));
		}

		void CreateImageViews(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectMask, VkImageView& outImageView)
		{
			VkImageViewCreateInfo                           createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = image;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = format;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = aspectMask;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			VK_CHECK_RESULT(vkCreateImageView(device, &createInfo, nullptr, &outImageView));
		}
    }
}