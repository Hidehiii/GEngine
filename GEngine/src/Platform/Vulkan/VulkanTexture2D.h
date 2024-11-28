#pragma once

#include "GEngine/Renderer/Texture.h"
#include <vulkan/vulkan.h>

namespace GEngine
{
	class GENGINE_API VulkanTexture2D : public Texture2D
	{
	public:
		VulkanTexture2D(const std::string& path);
		VulkanTexture2D(uint32_t width, uint32_t height, Image2DFormat format = Image2DFormat::R8G8B8A8F);
		VulkanTexture2D(uint32_t width, uint32_t height, void* data, uint32_t size, Image2DFormat format = Image2DFormat::R8G8B8A8F);
		VulkanTexture2D(VkFormat format, VkImage image, VkImageView imageView, VkDeviceMemory imageMemory, VkImageLayout layout, VkFlags aspectFlag, bool isMultiSample = false);
		virtual ~VulkanTexture2D();

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual void SetData(void* data, uint32_t size) override;
		virtual void Bind(const uint32_t slot = 0) override;
		virtual void Unbind() override;
		virtual std::string GetPath() const override { return m_Path; };

		// VK的相等不是比较ID，而是比较是否是同一个对象
		virtual bool operator==(const Texture& other) const override { return m_Image == ((VulkanTexture2D&)other).m_Image; };
		const VkSampler						GetSampler() { return m_Sampler; }
		const VkImageView					GetImageView() { return m_ImageView; }
		const VkDescriptorImageInfo			GetDescriptorImageInfo() { return m_ImageInfo; }
		const VkImageLayout					GetImageLayout() { return m_ImageLayout; }
		void SetImageLayout(VkImageLayout newLayout);
	public:
		// vulkan glsl 中的layout(binding)要用
		// 由于现在不知道会有多少
		static const uint32_t s_Texture2DBindingOffset = 10;
	private:
		void CreateSampler();
	private:
		std::string						m_Path;
		Image2DFormat					m_Format;
		uint32_t						m_Width, m_Height;
		VkFormat						m_DataFormat;
		VkImage							m_Image = nullptr;
		VkDeviceMemory					m_ImageMemory = nullptr;
		VkImageView						m_ImageView = nullptr;
		VkSampler						m_Sampler = nullptr;
		VkDescriptorImageInfo			m_ImageInfo{};
		VkImageLayout 					m_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkFlags							m_AspectFlag = 0;
		bool							m_MultiSample = false;
	};
}


