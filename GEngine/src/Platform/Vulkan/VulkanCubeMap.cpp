#include "GEpch.h"
#include "VulkanCubeMap.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"
#include "stb_image.h"

namespace GEngine
{
	VulkanCubeMap::VulkanCubeMap(uint32_t width, uint32_t height, RenderImage2DFormat format)
	{
		m_Height = height;
		m_Width = width;
		m_Format = format;
		Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
			VulkanContext::Get()->GetDevice(),
			m_Width,
			m_Height,
			Utils::RenderImage2DFormatToVulkanFormat(m_Format),
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			6,
			VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
			m_Image,
			m_ImageMemory);
		Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), m_Image, Utils::RenderImage2DFormatToVulkanFormat(m_Format), VK_IMAGE_ASPECT_COLOR_BIT, m_ImageView);
		CreateSampler();
	}

	VulkanCubeMap::VulkanCubeMap(const std::string& rightPath, const std::string& leftPath, const std::string& topPath, const std::string& buttomPath, const std::string& backPath, const std::string& frontPath)
	{
		m_Path.push_back(rightPath);
		m_Path.push_back(leftPath);
		m_Path.push_back(topPath);
		m_Path.push_back(buttomPath);
		m_Path.push_back(backPath);
		m_Path.push_back(frontPath);
		int	width, height, channels;
		stbi_uc* data;
		stbi_set_flip_vertically_on_load(1);
		data = stbi_load(rightPath.c_str(), &width, &height, &channels, 0);
		GE_CORE_ASSERT(data, "Failed to load image!");
		m_Width = width;
		m_Height = height;
		if (channels == 4)
		{
			m_Format = RenderImage2DFormat::RGBA8F;
		}
		else if (channels == 3)
		{
			m_Format = RenderImage2DFormat::RGB8F;
		}
		stbi_image_free(data);
		Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
			VulkanContext::Get()->GetDevice(),
			m_Width,
			m_Height,
			Utils::RenderImage2DFormatToVulkanFormat(m_Format),
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			6,
			VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
			m_Image,
			m_ImageMemory);
		Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), m_Image, Utils::RenderImage2DFormatToVulkanFormat(m_Format), VK_IMAGE_ASPECT_COLOR_BIT, m_ImageView);
		CreateSampler();
		LoadImageData();
	}

	VulkanCubeMap::~VulkanCubeMap()
	{
		vkDeviceWaitIdle(VulkanContext::Get()->GetDevice());
		vkDestroyImageView(VulkanContext::Get()->GetDevice(), m_ImageView, nullptr);
		vkDestroyImage(VulkanContext::Get()->GetDevice(), m_Image, nullptr);
		vkFreeMemory(VulkanContext::Get()->GetDevice(), m_ImageMemory, nullptr);
		vkDestroySampler(VulkanContext::Get()->GetDevice(), m_Sampler, nullptr);
	}

	void VulkanCubeMap::SetData(const void* data, uint32_t size)
	{
		GE_CORE_ASSERT(size == 6 * m_Width * m_Height * RenderImage2DFormatChannelSize(m_Format), "data size is not correct");
		for (int i = 0; i < 6; i++)
		{
			uint32_t tempSize = m_Width * m_Height * RenderImage2DFormatChannelSize(m_Format);
			void* tempData = new std::byte[tempSize];
			memcpy(tempData, ((std::byte*)data) + tempSize, tempSize);
			SetData(tempData, tempSize, (CubeMapFace)i);
			delete[](std::byte*)tempData;
		}
	}

	void VulkanCubeMap::SetData(const void* data, uint32_t size, CubeMapFace face)
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
		Utils::CopyBufferToImage(buffer, m_Image, m_Width, m_Height, 0, (uint32_t)face);
		SetImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(VulkanContext::Get()->GetDevice(), buffer, nullptr);
		vkFreeMemory(VulkanContext::Get()->GetDevice(), memory, nullptr);
	}

	void VulkanCubeMap::Bind(const uint32_t slot)
	{
		SetImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		m_ImageInfo.imageLayout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_ImageInfo.imageView		= m_ImageView;
		m_ImageInfo.sampler			= m_Sampler;
	}

	std::string VulkanCubeMap::GetPath() const
	{
		std::string path;
		for (auto str : m_Path)
		{
			path += "(" + str + ")";
		}
		return path;
	}

	void VulkanCubeMap::SetImageLayout(VkImageLayout newLayout)
	{
		Utils::TransitionImageLayout(m_Image, Utils::RenderImage2DFormatToVulkanFormat(m_Format), m_ImageLayout, newLayout, m_AspectFlag);
		m_ImageLayout = newLayout;
	}

	void VulkanCubeMap::CreateSampler()
	{
		VkSamplerCreateInfo             samplerInfo{};
		samplerInfo.sType				= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter			= VK_FILTER_NEAREST;
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
		samplerInfo.maxLod					= 0.0f;

		VK_CHECK_RESULT(vkCreateSampler(VulkanContext::Get()->GetDevice(), &samplerInfo, nullptr, &m_Sampler));
	}
	void VulkanCubeMap::LoadImageData()
	{
		for (int i = 0; i < m_Path.size(); i++)
		{
			int	width, height, channels;
			stbi_uc* data;
			stbi_set_flip_vertically_on_load(1);
			data = stbi_load(m_Path.at(i).c_str(), &width, &height, &channels, 0);
			GE_CORE_ASSERT(data, "Failed to load image!");
			GE_CORE_ASSERT(m_Width == width, " The cube map image is not the same size!");
			GE_CORE_ASSERT(m_Height = height, " The cube map image is not the same size!");
			RenderImage2DFormat format = RenderImage2DFormat::None;
			if (channels == 4)
			{
				format = RenderImage2DFormat::RGBA8F;
			}
			else if (channels == 3)
			{
				format = RenderImage2DFormat::RGB8F;
			}
			GE_CORE_ASSERT(m_Format == format, " The cube map image is not the same format!");
			SetData(data, m_Width * m_Height * RenderImage2DFormatChannelSize(m_Format), (CubeMapFace)i);
			stbi_image_free(data);
		}
	}
}