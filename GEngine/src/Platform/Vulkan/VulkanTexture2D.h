#pragma once

#include "GEngine/Renderer/Texture.h"
#include <vulkan/vulkan.h>

namespace GEngine
{
	class GENGINE_API VulkanTexture2D : public Texture2D
	{
	public:
		VulkanTexture2D(const std::string& path);
		VulkanTexture2D(uint32_t width, uint32_t height);
		VulkanTexture2D(uint32_t width, uint32_t height, void* data, uint32_t size);
		VulkanTexture2D(VkFormat format, VkImage image, VkImageView imageView);
		virtual ~VulkanTexture2D();

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual void SetData(void* data, uint32_t size) override;
		virtual void Bind(const uint32_t slot = 0) override;
		virtual void Unbind() override;
		virtual std::string GetPath() const override { return m_Path; };

		// VK的相等不是比较ID，而是比较是否是同一个对象
		virtual bool operator==(const Texture& other) const override { return m_Image == ((VulkanTexture2D&)other).m_Image; };
		VkDescriptorImageInfo GetDescriptorImageInfo() { return m_ImageInfo; }
		VkDescriptorSetLayoutBinding GetDescriptorSetLayoutBinding() { return m_DescriptorSetLayoutBinding; }
		uint32_t GetBinding() { return m_Binding; }
		void SetImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
	public:
		// vulkan glsl 中的layout(binding)要用
		// 由于现在不知道会有多少
		static const uint32_t s_Texture2DBindingOffset = 10;
	private:
		void CreateSampler();
	private:
		std::string						m_Path;
		uint32_t						m_Width, m_Height;
		uint32_t						m_RendererID;
		uint32_t						m_Binding;
		VkBuffer						m_Buffer;
		VkDeviceMemory					m_BufferMemory;
		VkFormat						m_DataFormat;
		VkImage							m_Image;
		VkDeviceMemory					m_ImageMemory;
		VkImageView						m_ImageView;
		VkSampler						m_Sampler;
		VkDescriptorSetLayoutBinding	m_DescriptorSetLayoutBinding;
		VkDescriptorImageInfo			m_ImageInfo{};
	};
}


