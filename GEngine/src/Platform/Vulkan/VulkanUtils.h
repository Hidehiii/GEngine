#pragma once
#include <vulkan/vulkan.h>
#include "GEngine/Renderer/FrameBuffer.h"

#ifdef GE_PLATFORM_WINDOWS
#define VK_CHECK_RESULT(f)																								\
	{																													\
		VkResult res = f;																								\
		GE_CORE_ASSERT(res == VK_SUCCESS, "Fatal : VkResult is {}\nIn File : {}\nAt Line :  {}\nStatement : {}",		\
				res, __FILE__, __LINE__, VAR_NAME(f));																	\
	}
#endif

namespace GEngine
{
	namespace Utils
	{
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDvice);
		void CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CreateImages(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& outImage, VkDeviceMemory& imageMemory);
		void CreateImageViews(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectMask, VkImageView& outImageView);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT);
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkAttachmentDescription CreateAttachmentDescription(FrameBufferTextureFormat format, int samples = 1);
		VkAttachmentReference CreateAttachmentReference(FrameBufferTextureFormat format, int index);
	}
	
}

