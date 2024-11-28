#pragma once
#include "GEngine/Compute/StorageImage.h"
#include <vulkan/vulkan.h>

namespace GEngine
{
	class GENGINE_API VulkanStorageImage2D : public StorageImage2D
	{
	public:
		VulkanStorageImage2D(uint32_t width, uint32_t height, Image2DFormat format);
	public:
		// vulkan glsl 中的layout(binding)要用
		// 由于现在不知道会有多少
		static const uint32_t s_StorageImage2DBindingOffset = 20;
	private:
		void CreateSampler();
	private:
		uint32_t						m_Width, m_Height;
		Image2DFormat					m_Format;
		VkImage							m_Image = nullptr;
		VkDeviceMemory					m_ImageMemory = nullptr;
		VkImageView						m_ImageView = nullptr;
		VkSampler						m_Sampler = nullptr;
		VkDescriptorImageInfo			m_ImageInfo{};
		VkImageLayout 					m_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkFlags							m_AspectFlag = 0;
	};
}


