#pragma once
#include "GEngine/Renderer/Texture.h"
#include <vulkan/vulkan.h>

namespace GEngine
{
	class GENGINE_API VulkanCubeMap : public CubeMap
	{
	public:
		VulkanCubeMap(uint32_t width, uint32_t height, bool generateMipmap = false, RenderImage2DFormat format = RenderImage2DFormat::RGBA8F);
		VulkanCubeMap(const std::string& rightPath, const std::string& leftPath, const std::string& topPath, const std::string& buttomPath, const std::string& backPath, const std::string& frontPath, bool generateMipmap = false);
		virtual ~VulkanCubeMap();

		virtual void Bind(const uint32_t slot = 0) override;

		virtual std::string GetPath() const override;

		virtual void SetData(const void* data, uint32_t size) override;
		virtual void SetData(const void* data, uint32_t size, CubeMapFace face) override;
		virtual void SetData(const Ref<Texture2D>& texture, uint32_t width, uint32_t height, CubeMapFace face) override;
		
		
		virtual bool operator==(const Texture& other) const override { return m_Image == ((VulkanCubeMap&)other).m_Image; };

		const VkSampler						GetSampler() { return m_Sampler; }
		const VkImageView					GetImageView() { return m_ImageView; }
		const VkDescriptorImageInfo*		GetDescriptorImageInfo() { return &m_ImageInfo; }
		const VkImageLayout					GetImageLayout() { return m_ImageLayout; }

		void SetImageLayout(VkImageLayout newLayout);
		void SetImageLayout(VkCommandBuffer cmdBuffer, VkImageLayout newLayout);
	private:
		void CreateSampler();
		void LoadImageData();
	private:
		std::vector<std::string>		m_Path;
		VkImage							m_Image = nullptr;
		VkDeviceMemory					m_ImageMemory = nullptr;
		VkImageView						m_ImageView = nullptr;
		VkSampler						m_Sampler = nullptr;
		VkDescriptorImageInfo			m_ImageInfo{};
		VkImageLayout 					m_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkFlags							m_AspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
	};
}


