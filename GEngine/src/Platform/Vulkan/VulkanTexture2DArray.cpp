#include "GEpch.h"
#include "VulkanTexture2DArray.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanTexture2D.h"

namespace GEngine
{
    VulkanTexture2DArray::VulkanTexture2DArray(uint32_t width, uint32_t height, uint32_t layers, RenderImage2DFormat format)
    {
		m_Height    = height;
		m_Width     = width;
		m_Format    = format;
        m_Layers    = layers;
		if (m_GenerateMipmap)
		{
			m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;
		}
		Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
			VulkanContext::Get()->GetDevice(),
			m_Width,
			m_Height,
			Utils::RenderImage2DFormatToVulkanFormat(m_Format),
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			m_Layers,
			0,
			m_MipLevels,
			m_Image,
			m_ImageMemory);
		Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), m_Image, Utils::RenderImage2DFormatToVulkanFormat(m_Format), VK_IMAGE_VIEW_TYPE_2D_ARRAY, m_Layers, m_AspectFlag, m_MipLevels, m_ImageView);
		CreateSampler();
    }
    VulkanTexture2DArray::~VulkanTexture2DArray()
    {
		if (VulkanContext::Get()->GetDevice())
		{
			vkDeviceWaitIdle(VulkanContext::Get()->GetDevice());
			vkDestroyImageView(VulkanContext::Get()->GetDevice(), m_ImageView, nullptr);
			vkDestroyImage(VulkanContext::Get()->GetDevice(), m_Image, nullptr);
			vkFreeMemory(VulkanContext::Get()->GetDevice(), m_ImageMemory, nullptr);
			vkDestroySampler(VulkanContext::Get()->GetDevice(), m_Sampler, nullptr);
		}
    }
	void VulkanTexture2DArray::Bind(CommandBuffer* cmdBuffer, const uint32_t slot)
	{
		SetImageLayout(((VulkanCommandBuffer*)cmdBuffer)->GetCommandBuffer(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		m_ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_ImageInfo.imageView	= m_ImageView;
		m_ImageInfo.sampler		= m_Sampler;
	}
	void VulkanTexture2DArray::SetData(const void* data, uint32_t size)
	{
		uint32_t tempSize = m_Width * m_Height * RenderImage2DFormatChannelSize(m_Format);
		for (uint32_t layer = 0; layer < size / tempSize; layer++)
		{
			uint32_t offset = tempSize * layer;
			void* tempData = new std::byte[tempSize];
			memcpy(tempData, ((std::byte*)data) + offset, tempSize);
			SetData(tempData, tempSize, layer);
			delete[](std::byte*)tempData;
		}
	}
	void VulkanTexture2DArray::SetData(const Ref<Texture2D>& texture, uint32_t width, uint32_t height, uint32_t layer)
	{
		SetImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		std::dynamic_pointer_cast<VulkanTexture2D>(texture)->SetImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		Utils::CopyImageToImage(width, height, std::dynamic_pointer_cast<VulkanTexture2D>(texture)->GetImage(),
			std::dynamic_pointer_cast<VulkanTexture2D>(texture)->GetImageLayout(),
			std::dynamic_pointer_cast<VulkanTexture2D>(texture)->GetAspectFlag(),
			0, 0,
			m_Image, m_ImageLayout, m_AspectFlag,
			0, layer);

		if (m_GenerateMipmap)
		{
			Utils::GenerateMipmap(m_Image, m_Width, m_Height, m_MipLevels, m_AspectFlag, 0, m_Layers);
			m_ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
		else
		{
			SetImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	}
	void VulkanTexture2DArray::SetData(const void* data, uint32_t size, uint32_t layer)
	{
		VkBuffer    buffer;
		VkDeviceMemory memory;

		Utils::CreateBuffer(VulkanContext::Get()->GetPhysicalDevice(),
			VulkanContext::Get()->GetDevice(),
			static_cast<uint32_t>(size),
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			buffer,
			memory);

		void* tempData;
		vkMapMemory(VulkanContext::Get()->GetDevice(), memory, 0, size, 0, &tempData);
		memcpy(tempData, data, static_cast<uint32_t>(size));
		vkUnmapMemory(VulkanContext::Get()->GetDevice(), memory);
		SetImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		Utils::CopyBufferToImage(buffer, m_Image, m_Width, m_Height, 0, layer, m_AspectFlag);

		vkDestroyBuffer(VulkanContext::Get()->GetDevice(), buffer, nullptr);
		vkFreeMemory(VulkanContext::Get()->GetDevice(), memory, nullptr);
		if (m_GenerateMipmap)
		{
			Utils::GenerateMipmap(m_Image, m_Width, m_Height, m_MipLevels, m_AspectFlag, 0, m_Layers);
			m_ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
		else
		{
			SetImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	}
	void VulkanTexture2DArray::SetImageLayout(VkImageLayout newLayout)
	{
		Utils::TransitionImageLayout(m_Image, Utils::RenderImage2DFormatToVulkanFormat(m_Format), m_ImageLayout, newLayout, 6, m_AspectFlag, m_MipLevels);
		m_ImageLayout = newLayout;
	}
	void VulkanTexture2DArray::SetImageLayout(VkCommandBuffer cmdBuffer, VkImageLayout newLayout)
	{
		Utils::TransitionImageLayout(cmdBuffer, m_Image, Utils::RenderImage2DFormatToVulkanFormat(m_Format), m_ImageLayout, newLayout, 6, m_AspectFlag, m_MipLevels);
		m_ImageLayout = newLayout;
	}
	void VulkanTexture2DArray::CreateSampler()
	{
		VkSamplerCreateInfo             samplerInfo{};
		samplerInfo.sType				= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter			= VK_FILTER_LINEAR;
		samplerInfo.minFilter			= VK_FILTER_LINEAR;
		samplerInfo.addressModeU		= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV		= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW		= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable	= VK_TRUE;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(VulkanContext::Get()->GetPhysicalDevice(), &properties);

		samplerInfo.maxAnisotropy			= properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor				= VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable			= VK_FALSE;
		samplerInfo.compareOp				= VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode				= VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias				= 0.0f;
		samplerInfo.minLod					= 0.0f;
		samplerInfo.maxLod					= VK_LOD_CLAMP_NONE;

		VK_CHECK_RESULT(vkCreateSampler(VulkanContext::Get()->GetDevice(), &samplerInfo, nullptr, &m_Sampler));
	}
}