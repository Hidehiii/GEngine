#pragma once
#include <vulkan/vulkan.h>
#include "GEngine/Renderer/FrameBuffer.h"
#include "GEngine/Renderer/ImageFormat.h"
#include "GEngine/Renderer/Shader.h"
#include "GEngine/Renderer/Sampler.h"

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
		uint32_t					FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDvice);
		VkFormat					FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		
		void						TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layerCount, VkFlags aspectFlag, uint32_t mipLevel);
		void						TransitionImageLayout(VkCommandBuffer CmdBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layerCount, VkFlags aspectFlag, uint32_t mipLevel);

		void BlitImage(VkImage src, VkImageLayout srcLayout, Vector2 srcSize, uint32_t srcMipLevel, VkImage dst, VkImageLayout dstLayout, Vector2 dstSize, uint32_t dstMipLevel, VkImageAspectFlags aspectFlag);
		void BlitImage(VkCommandBuffer CmdBuffer, VkImage src, VkImageLayout srcLayout, Vector2 srcSize, uint32_t srcMipLevel, VkImage dst, VkImageLayout dstLayout, Vector2 dstSize, uint32_t dstMipLevel, VkImageAspectFlags aspectFlag);
		
		void CopyBufferToBuffer(VkBuffer src, VkBuffer dst, uint32_t size);
		void CopyBufferToBuffer(VkCommandBuffer CmdBuffer, VkBuffer src, VkBuffer dst, uint32_t size);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t mipLevel, uint32_t baseArrayLayer, VkFlags aspectFlag);
		void CopyImageToImage(uint32_t width, uint32_t height, VkImage src, VkImageLayout srcLayout, VkImageAspectFlags srcAspectFlag, uint32_t srcMipLevel, uint32_t srcBaseArrayLayer, VkImage dst, VkImageLayout dstLayout, VkImageAspectFlags dstAspectFlag, uint32_t dstMipLevel, uint32_t dstBaseArrayLayer);

		void GenerateMipmap(VkImage image, uint32_t width, uint32_t height, uint32_t mipLevel, VkImageAspectFlags aspectFlag, uint32_t baseArrayLayer, uint32_t layerCount);
		
		void							CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void							CreateImages(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkSampleCountFlagBits sample, uint32_t arrayLayers, VkImageCreateFlags flags, uint32_t mipLevel, VkImage& outImage, VkDeviceMemory& imageMemory);
		void							CreateImageViews(VkDevice device, VkImage image, VkFormat format, VkImageViewType viewType, uint32_t layerCount, VkImageAspectFlags aspectMask, uint32_t mipLevel, VkImageView& outImageView);
		VkAttachmentDescription2		CreateAttachmentDescription2(FrameBufferTextureFormat format, VkSampleCountFlagBits sample, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp);
		VkAttachmentReference2			CreateAttachmentReference2(FrameBufferTextureFormat format, int index);
		VkAttachmentDescription			CreateAttachmentDescription(FrameBufferTextureFormat format, VkSampleCountFlagBits sample, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp);
		VkAttachmentReference			CreateAttachmentReference(FrameBufferTextureFormat format, int index);
		VkPipelineShaderStageCreateInfo	CreatePipelineShaderStage(VkShaderStageFlagBits stage, VkShaderModule module, const char* funcName);

		VkSampleCountFlagBits	SampleCountToVulkanFlag(int sample);
		VkFormat				RenderImage2DFormatToVulkanFormat(RenderImage2DFormat format);
		VkFormat				ComputeImage2DFormatToVulkanFormat(ComputeImage2DFormat format);
		RenderImage2DFormat		VulkanFormatToRenderImage2DFormat(VkFormat format);
		VkBlendFactor			BlendFactorToVulkanBlendFactor(BlendFactor factor);
		VkCullModeFlagBits		CullModeToVkCullMode(CullMode mode);
		VkCompareOp				CompareOPToVkCompareOP(CompareOperation op);
		VkAttachmentLoadOp		AttachmentsActionToVkAttachmentLoadOp(FrameBufferAttachmentsAction op);
		VkAttachmentStoreOp		AttachmentsActionToVkAttachmentStoreOp(FrameBufferAttachmentsAction op);
		VkFilter				SamplerFilterToVkFilter(SamplerFilter filter);
		VkSamplerAddressMode	SamplerAddressModeToVkSamplerAddressMode(SamplerAddressMode mode);
		VkSamplerMipmapMode		SamplerFilterToVkSamplerMipmapMode(SamplerFilter filter);
	}
	
}

