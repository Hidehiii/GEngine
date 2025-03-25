#pragma once
#include "GEngine/Graphics/Texture.h"
#include <vulkan/vulkan.h>

namespace GEngine
{
	class GENGINE_API VulkanTexture2DArray : public Texture2DArray
	{
	public:
		VulkanTexture2DArray(uint32_t width, uint32_t height, uint32_t layers, RenderImage2DFormat format);
		virtual ~VulkanTexture2DArray() override;

		virtual void Bind(CommandBuffer* cmdBuffer, const uint32_t slot = 0) override;

		virtual void SetData(const void* data, uint32_t size) override;
		virtual void SetData(const Ref<Texture2D>& texture, uint32_t width, uint32_t height, uint32_t layer) override;
		virtual void SetData(const void* data, uint32_t size, uint32_t layer) override;

		virtual std::string GetPath() const override { return "NULL"; }

		virtual bool operator==(const Texture2DArray& other) const override { return m_Image == ((VulkanTexture2DArray&)other).m_Image; };

		const VkSampler						GetSampler() { return m_Sampler; }
		const VkImageView					GetImageView() { return m_ImageView; }
		const VkDescriptorImageInfo*		GetDescriptorImageInfo() { return &m_ImageInfo; }
		const VkImageLayout					GetImageLayout() { return m_ImageLayout; }

		void SetImageLayout(VkImageLayout newLayout);
		void SetImageLayout(VkCommandBuffer cmdBuffer, VkImageLayout newLayout);
	private:
		void CreateSampler();
	private:
		VkImage							m_Image;
		VkDeviceMemory					m_ImageMemory = nullptr;
		VkImageView						m_ImageView = nullptr;
		VkSampler						m_Sampler = nullptr;
		VkDescriptorImageInfo			m_ImageInfo{};
		VkImageLayout 					m_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkFlags							m_AspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
	};

}

