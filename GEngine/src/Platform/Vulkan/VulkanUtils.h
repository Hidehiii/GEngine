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
		void CreateImages(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkSampleCountFlagBits sample, uint32_t arrayLayers, VkImageCreateFlags flags, uint32_t mipLevel, VkImage& outImage, VkDeviceMemory& imageMemory);
		void CreateImageViews(VkDevice device, VkImage image, VkFormat format, VkImageViewType viewType, uint32_t layerCount, VkImageAspectFlags aspectMask, uint32_t mipLevel, VkImageView& outImageView);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layerCount, VkFlags aspectFlag, uint32_t mipLevel);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t mipLevel, uint32_t baseArrayLayer, VkFlags aspectFlag);
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkAttachmentDescription2 CreateAttachmentDescription2(FrameBufferTextureFormat format, VkSampleCountFlagBits sample);
		VkAttachmentReference2 CreateAttachmentReference2(FrameBufferTextureFormat format, int index);
		VkAttachmentDescription CreateAttachmentDescription(FrameBufferTextureFormat format, VkSampleCountFlagBits sample);
		VkAttachmentReference CreateAttachmentReference(FrameBufferTextureFormat format, int index);
		VkSampleCountFlagBits SampleCountToVulkanFlag(int sample);
		void BlitImage(VkImage src, VkImageLayout srcLayout, Vector2 srcSize, uint32_t srcMipLevel, VkImage dst, VkImageLayout dstLayout, Vector2 dstSize, uint32_t dstMipLevel, VkImageAspectFlags aspectFlag);
		VkFormat RenderImage2DFormatToVulkanFormat(RenderImage2DFormat format);
		VkFormat ComputeImage2DFormatToVulkanFormat(ComputeImage2DFormat format);
		RenderImage2DFormat VulkanFormatToRenderImage2DFormat(VkFormat format);
		void CopyBufferToBuffer(VkBuffer src, VkBuffer dst, uint32_t size);
		VkBlendFactor BlendFactorToVulkanBlendFactor(BlendFactor factor);
		VkCullModeFlagBits CullModeToVkCullMode(CullMode mode);
		void CopyImageToImage(uint32_t width, uint32_t height, VkImage src, VkImageLayout srcLayout, VkImageAspectFlags srcAspectFlag, uint32_t srcMipLevel, uint32_t srcBaseArrayLayer, VkImage dst, VkImageLayout dstLayout, VkImageAspectFlags dstAspectFlag, uint32_t dstMipLevel, uint32_t dstBaseArrayLayer);
		void GenerateMipmap(VkImage image, uint32_t width, uint32_t height, uint32_t mipLevel, VkImageAspectFlags aspectFlag, uint32_t baseArrayLayer, uint32_t layerCount);
		VkCompareOp CompareOPToVkCompareOP(CompareOperation op);
	}
	
}

