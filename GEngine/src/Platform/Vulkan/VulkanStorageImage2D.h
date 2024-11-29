#pragma once
#include "GEngine/Compute/StorageImage.h"
#include <vulkan/vulkan.h>

namespace GEngine
{
	class GENGINE_API VulkanStorageImage2D : public StorageImage2D
	{
	public:
		VulkanStorageImage2D(uint32_t width, uint32_t height, ComputeImage2DFormat format);
		virtual ~VulkanStorageImage2D();

		virtual void SetData(const void* data, uint32_t size) override;
		virtual void Bind(const uint32_t slot = 0) override;

		const VkImageView					GetImageView() { return m_ImageView; }
		const VkDescriptorImageInfo			GetDescriptorImageInfo() { return m_ImageInfo; }
		const VkImageLayout					GetImageLayout() { return m_ImageLayout; }
		void SetImageLayout(VkImageLayout newLayout);
	private:
		uint32_t						m_Width, m_Height;
		ComputeImage2DFormat			m_Format;
		VkImage							m_Image = nullptr;
		VkDeviceMemory					m_ImageMemory = nullptr;
		VkImageView						m_ImageView = nullptr;
		VkDescriptorImageInfo			m_ImageInfo{};
		VkImageLayout 					m_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkFlags							m_AspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
	};
}


