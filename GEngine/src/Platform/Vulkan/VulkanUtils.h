#pragma once
#include <vulkan/vulkan.h>
#include "GEngine/Renderer/FrameBuffer.h"
#include "GEngine/Renderer/ImageFormat.h"
#include "GEngine/Renderer/Shader.h"

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
		void CreateImages(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkSampleCountFlagBits sample, uint32_t arrayLayers, VkImageCreateFlags flags, VkImage& outImage, VkDeviceMemory& imageMemory);
		void CreateImageViews(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectMask, VkImageView& outImageView);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT);
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkAttachmentDescription2 CreateAttachmentDescription2(FrameBufferTextureFormat format, VkSampleCountFlagBits sample = VK_SAMPLE_COUNT_1_BIT);
		VkAttachmentReference2 CreateAttachmentReference2(FrameBufferTextureFormat format, int index);
		VkAttachmentDescription CreateAttachmentDescription(FrameBufferTextureFormat format, VkSampleCountFlagBits sample = VK_SAMPLE_COUNT_1_BIT);
		VkAttachmentReference CreateAttachmentReference(FrameBufferTextureFormat format, int index);
		VkSampleCountFlagBits SampleCountToVulkanFlag(int sample);
		void BlitImage(VkImage src, VkImageLayout srcLayout, Vector2 srcSize, VkImage dst, VkImageLayout dstLayout, Vector2 dstSize, VkImageAspectFlags aspectFlag);
		VkFormat RenderImage2DFormatToVulkanFormat(RenderImage2DFormat format);
		VkFormat ComputeImage2DFormatToVulkanFormat(ComputeImage2DFormat format);
		RenderImage2DFormat VulkanFormatToRenderImage2DFormat(VkFormat format);
		void CopyBufferToBuffer(VkBuffer src, VkBuffer dst, uint32_t size);
		VkBlendFactor BlendFactorToVulkanBlendFactor(BlendFactor factor);
		VkCullModeFlagBits CullModeToVkCullMode(CullMode mode);
	}
	
}

