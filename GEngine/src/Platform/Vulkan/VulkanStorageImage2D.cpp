#include "GEpch.h"
#include "VulkanStorageImage2D.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "VulkanUtils.h"

namespace GEngine
{

	VulkanStorageImage2D::VulkanStorageImage2D(uint32_t width, uint32_t height, ComputeImage2DFormat format)
	{
		m_Height = height;
		m_Width = width;
		m_Format = format;
		Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
			VulkanContext::Get()->GetDevice(),
			m_Width,
			m_Height,
			Utils::ComputeImage2DFormatToVulkanFormat(m_Format),
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			1,
			0,
			m_Image,
			m_ImageMemory);
		Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), m_Image, Utils::ComputeImage2DFormatToVulkanFormat(m_Format), VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, m_ImageView);
	}

	VulkanStorageImage2D::~VulkanStorageImage2D()
	{
		vkDeviceWaitIdle(VulkanContext::Get()->GetDevice());
		vkDestroyImageView(VulkanContext::Get()->GetDevice(), m_ImageView, nullptr);
		vkDestroyImage(VulkanContext::Get()->GetDevice(), m_Image, nullptr);
		vkFreeMemory(VulkanContext::Get()->GetDevice(), m_ImageMemory, nullptr);
	}

	void VulkanStorageImage2D::Bind(const uint32_t slot)
	{
		SetImageLayout(VK_IMAGE_LAYOUT_GENERAL);
		m_ImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		m_ImageInfo.imageView	= m_ImageView;
	}

	void VulkanStorageImage2D::SetImageLayout(VkImageLayout newLayout)
	{
		Utils::TransitionImageLayout(m_Image, Utils::ComputeImage2DFormatToVulkanFormat(m_Format), m_ImageLayout, newLayout, m_AspectFlag);
		m_ImageLayout = newLayout;
	}


}