#include "GEpch.h"
#include "VulkanStorageImage2D.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "VulkanUtils.h"

namespace GEngine
{

	VulkanStorageImage2D::VulkanStorageImage2D(uint32_t width, uint32_t height, Image2DFormat format)
	{
		m_Height = height;
		m_Width = width;
		m_Format = format;
		Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
			VulkanContext::Get()->GetDevice(),
			m_Width,
			m_Height,
			Utils::Image2DFormatToVulkanFormat(m_Format),
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			m_Image,
			m_ImageMemory);
		Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), m_Image, Utils::Image2DFormatToVulkanFormat(m_Format), VK_IMAGE_ASPECT_COLOR_BIT, m_ImageView);

		CreateSampler();
	}

	void VulkanStorageImage2D::CreateSampler()
	{
		VkSamplerCreateInfo             samplerInfo{};
		samplerInfo.sType				= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter			= VK_FILTER_LINEAR;
		samplerInfo.minFilter			= VK_FILTER_LINEAR;
		samplerInfo.addressModeU		= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV		= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW		= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable	= VK_TRUE;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(VulkanContext::Get()->GetPhysicalDevice(), &properties);

		samplerInfo.maxAnisotropy			= properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor				= VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable			= VK_FALSE;
		samplerInfo.compareOp				= VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode				= VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias				= 0.0f;
		samplerInfo.minLod					= 0.0f;
		samplerInfo.maxLod					= 0.0f;

		VK_CHECK_RESULT(vkCreateSampler(VulkanContext::Get()->GetDevice(), &samplerInfo, nullptr, &m_Sampler));
	}
}