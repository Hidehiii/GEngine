#include "GEpch.h"
#include "VulkanCubeMap.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"
#include "stb_image.h"

namespace GEngine
{
	VulkanCubeMap::VulkanCubeMap(uint32_t width, uint32_t height, RenderImage2DFormat format)
	{
		m_Height = height;
		m_Width = width;
		m_Format = format;
		Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
			VulkanContext::Get()->GetDevice(),
			m_Width,
			m_Height,
			Utils::RenderImage2DFormatToVulkanFormat(m_Format),
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			6,
			VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
			m_Image,
			m_ImageMemory);
		Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), m_Image, Utils::RenderImage2DFormatToVulkanFormat(m_Format), VK_IMAGE_ASPECT_COLOR_BIT, m_ImageView);
		CreateSampler();
	}

	VulkanCubeMap::VulkanCubeMap(const std::string& rightPath, const std::string& leftPath, const std::string& topPath, const std::string& buttomPath, const std::string& backPath, const std::string& frontPath)
	{
	}

	VulkanCubeMap::~VulkanCubeMap()
	{
		vkDeviceWaitIdle(VulkanContext::Get()->GetDevice());
		vkDestroyImageView(VulkanContext::Get()->GetDevice(), m_ImageView, nullptr);
		vkDestroyImage(VulkanContext::Get()->GetDevice(), m_Image, nullptr);
		vkFreeMemory(VulkanContext::Get()->GetDevice(), m_ImageMemory, nullptr);
		vkDestroySampler(VulkanContext::Get()->GetDevice(), m_Sampler, nullptr);
	}

	void VulkanCubeMap::SetData(void* data, uint32_t size)
	{
	}

	void VulkanCubeMap::SetData(void* data, uint32_t size, CubeMapFace face)
	{
	}

	void VulkanCubeMap::SetImageLayout(VkImageLayout newLayout)
	{
		Utils::TransitionImageLayout(m_Image, Utils::RenderImage2DFormatToVulkanFormat(m_Format), m_ImageLayout, newLayout, m_AspectFlag);
		m_ImageLayout = newLayout;
	}

	void VulkanCubeMap::CreateSampler()
	{
		VkSamplerCreateInfo             samplerInfo{};
		samplerInfo.sType				= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter			= VK_FILTER_NEAREST;
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