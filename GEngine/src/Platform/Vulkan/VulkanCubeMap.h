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

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual void SetData(void* data, uint32_t size) override;
		virtual void SetData(void* data, uint32_t size, CubeMapFace face) override;
		virtual bool operator==(const Texture& other) const override { return m_Image == ((VulkanCubeMap&)other).m_Image; };

		const VkSampler						GetSampler() { return m_Sampler; }
		const VkImageView					GetImageView() { return m_ImageView; }
		const VkDescriptorImageInfo			GetDescriptorImageInfo() { return m_ImageInfo; }
		const VkImageLayout					GetImageLayout() { return m_ImageLayout; }
		void SetImageLayout(VkImageLayout newLayout);
	private:
		void CreateSampler();
	private:
		std::vector<std::string>		m_Path;
		RenderImage2DFormat				m_Format;
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


