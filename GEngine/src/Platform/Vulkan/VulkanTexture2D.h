#pragma once

#include "GEngine/Renderer/Texture.h"
#include <vulkan/vulkan.h>

namespace GEngine
{
	class GENGINE_API VulkanTexture2D : public Texture2D
	{
	public:
		VulkanTexture2D(const std::string& path);
		VulkanTexture2D(uint32_t width, uint32_t height, RenderImage2DFormat format = RenderImage2DFormat::RGBA8F);
		VulkanTexture2D(uint32_t width, uint32_t height, void* data, uint32_t size, RenderImage2DFormat format = RenderImage2DFormat::RGBA8F);
		VulkanTexture2D(VkFormat format, VkImage image, VkImageView imageView, VkDeviceMemory imageMemory, VkImageLayout layout, VkFlags aspectFlag);
		virtual ~VulkanTexture2D();

		virtual void Bind(CommandBuffer* cmdBuffer, const uint32_t slot = 0) override;

		virtual std::string GetPath() const override { return m_Path; };

		virtual void SetData(const void* data, uint32_t size) override;
		virtual void SetData(const Ref<Texture2D>& texture, uint32_t width, uint32_t height) override;

		// VK的相等不是比较ID，而是比较是否是同一个对象
		virtual bool operator==(const Texture2D& other) const override { return m_Image == ((VulkanTexture2D&)other).m_Image; };

		const VkImage						GetImage() { return m_Image; }
		const VkSampler						GetSampler() { return m_Sampler; }
		const VkImageView					GetImageView() { return m_ImageView; }
		const VkDescriptorImageInfo*		GetDescriptorImageInfo() { return &m_ImageInfo; }
		const VkImageLayout					GetImageLayout() { return m_ImageLayout; }
		const VkImageAspectFlags			GetAspectFlag() { return m_AspectFlag; }

		void SetImageLayout(VkImageLayout newLayout);
		void SetImageLayout(VkCommandBuffer cmdBuffer,  VkImageLayout newLayout);
	private:
		void CreateSampler();
	private:
		std::string						m_Path;
		VkImage							m_Image = nullptr;
		VkDeviceMemory					m_ImageMemory = nullptr;
		VkImageView						m_ImageView = nullptr;
		VkSampler						m_Sampler = nullptr;
		VkDescriptorImageInfo			m_ImageInfo{};
		VkImageLayout 					m_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkFlags							m_AspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
	};
}


