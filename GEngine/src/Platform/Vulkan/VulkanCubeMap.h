#pragma once
#include "GEngine/Renderer/Texture.h"
#include <vulkan/vulkan.h>

namespace GEngine
{
	class GENGINE_API VulkanCubeMap : public CubeMap
	{
	public:
		VulkanCubeMap(uint32_t width, uint32_t height, RenderImage2DFormat format = RenderImage2DFormat::RGBA8F);
		VulkanCubeMap(const std::string& rightPath, const std::string& leftPath, const std::string& topPath, const std::string& buttomPath, const std::string& backPath, const std::string& frontPath);
		virtual ~VulkanCubeMap();
	private:
		void CreateSampler();
	private:
		RenderImage2DFormat					m_Format;
		uint32_t						m_Width, m_Height;
		VkImage							m_Image = nullptr;
		VkDeviceMemory					m_ImageMemory = nullptr;
		VkImageView						m_ImageView = nullptr;
		VkSampler						m_Sampler = nullptr;
		VkDescriptorImageInfo			m_ImageInfo{};
		VkImageLayout 					m_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkFlags							m_AspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
	};
}


