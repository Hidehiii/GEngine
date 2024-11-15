#include "GEpch.h"
#include "VulkanTexture2D.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"
#include "stb_image.h"

namespace GEngine
{
    VulkanTexture2D::VulkanTexture2D(const std::string& path)
    {
        m_Path = path;
        int			width, height, channels;
        stbi_uc* data;
        stbi_set_flip_vertically_on_load(1);
        {
            GE_PROFILE_SCOPE("stbi_load - OpenGLTexture2D::OpenGLTexture2D(const std::string& path)")
            data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        }
        GE_CORE_ASSERT(data, "Failed to load image!");
        m_Width         = width;
        m_Height        = height;
        if (channels == 4)
        {
            m_DataFormat = VK_FORMAT_R8G8B8A8_UNORM;
        }
        else
        {
            m_DataFormat = VK_FORMAT_R8G8B8_UNORM;
        }

		Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
			                    VulkanContext::Get()->GetDevice(),
			                m_Width,
			                m_Height,
			                m_DataFormat,
			                VK_IMAGE_TILING_OPTIMAL,
			                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			                m_Image,
			                m_ImageMemory);
        Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), m_Image, m_DataFormat, VK_IMAGE_ASPECT_COLOR_BIT, m_ImageView);
        SetData(data, m_Width * m_Height * channels);
        stbi_image_free(data);
        
        CreateSampler();
    }
    VulkanTexture2D::VulkanTexture2D(uint32_t width, uint32_t height)
    {
        m_Height    = height;
        m_Width     = width;
        m_DataFormat = VK_FORMAT_R8G8B8A8_UNORM;
		Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
			                VulkanContext::Get()->GetDevice(),
			                m_Width,
			                m_Height,
			                m_DataFormat,
			                VK_IMAGE_TILING_OPTIMAL,
			                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			                m_Image,
			                m_ImageMemory);
		Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), m_Image, m_DataFormat, VK_IMAGE_ASPECT_COLOR_BIT, m_ImageView);

        CreateSampler();
    }
    VulkanTexture2D::VulkanTexture2D(uint32_t width, uint32_t height, void* data, uint32_t size)
    {
		m_Height = height;
		m_Width = width;
		m_DataFormat = VK_FORMAT_R8G8B8A8_UNORM;
		Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
			VulkanContext::Get()->GetDevice(),
			m_Width,
			m_Height,
			m_DataFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_Image,
			m_ImageMemory);
		Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), m_Image, m_DataFormat, VK_IMAGE_ASPECT_COLOR_BIT, m_ImageView);

		CreateSampler();
        SetData(data, size);
    }
    VulkanTexture2D::VulkanTexture2D(VkFormat format, VkImage image, VkImageView imageView, VkImageLayout layout, VkFlags aspectFlag, bool isAttachmentImage)
    {
        m_DataFormat    = format;
        m_Image         = image;
        m_ImageView     = imageView;
        m_ImageLayout   = layout;
        m_IsAttachmentImage = isAttachmentImage;
        m_AspectFlag = aspectFlag;
        if (m_IsAttachmentImage)
        {
            m_AttachmentImageLayout = layout;
        }
        CreateSampler();
    }
    VulkanTexture2D::~VulkanTexture2D()
    {
            vkDestroyImage(VulkanContext::Get()->GetDevice(), m_Image, nullptr);
            vkFreeMemory(VulkanContext::Get()->GetDevice(), m_ImageMemory, nullptr);
            vkDestroySampler(VulkanContext::Get()->GetDevice(), m_Sampler, nullptr);
            vkDestroyImageView(VulkanContext::Get()->GetDevice(), m_ImageView, nullptr);
        
    }
    void VulkanTexture2D::SetData(void* data, uint32_t size)
    {
        uint32_t bpp = m_DataFormat == VK_FORMAT_R8G8B8A8_UNORM ? 4 : 3;
        GE_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture!");

		Utils::CreateBuffer(VulkanContext::Get()->GetPhysicalDevice(),
			                VulkanContext::Get()->GetDevice(),
			                static_cast<uint32_t>(m_Width * m_Height * bpp),
			                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			                m_Buffer,
			                m_BufferMemory);
		

        void* tempData;
        vkMapMemory(VulkanContext::Get()->GetDevice(), m_BufferMemory, 0, size, 0, &tempData);
        memcpy(tempData, data, static_cast<size_t>(size));
        vkUnmapMemory(VulkanContext::Get()->GetDevice(), m_BufferMemory);

        Utils::TransitionImageLayout(m_Image, m_DataFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        Utils::CopyBufferToImage(m_Buffer, m_Image, m_Width, m_Height);
        Utils::TransitionImageLayout(m_Image, m_DataFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        m_ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        vkDestroyBuffer(VulkanContext::Get()->GetDevice(), m_Buffer, nullptr);
        vkFreeMemory(VulkanContext::Get()->GetDevice(), m_BufferMemory, nullptr);
    }
    void VulkanTexture2D::Bind(const uint32_t slot)
    {
		
		m_ImageInfo.imageLayout       = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_ImageInfo.imageView         = m_ImageView;
		m_ImageInfo.sampler           = m_Sampler;
    }
    void VulkanTexture2D::Unbind()
    {

    }
    void VulkanTexture2D::SetImageLayout(VkImageLayout newLayout)
    {
        Utils::TransitionImageLayout(m_Image, m_DataFormat, m_ImageLayout, newLayout);
        m_ImageLayout = newLayout;
    }
    void VulkanTexture2D::CreateSampler()
    {
		VkSamplerCreateInfo             samplerInfo{};
		samplerInfo.sType               = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter           = VK_FILTER_LINEAR;
		samplerInfo.minFilter           = VK_FILTER_LINEAR;
		samplerInfo.addressModeU        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable    = VK_TRUE;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(VulkanContext::Get()->GetPhysicalDevice(), &properties);

        samplerInfo.maxAnisotropy               = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor                 = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates     = VK_FALSE;
		samplerInfo.compareEnable               = VK_FALSE;
		samplerInfo.compareOp                   = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode                  = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias                  = 0.0f;
		samplerInfo.minLod                      = 0.0f;
		samplerInfo.maxLod                      = 0.0f;

        VK_CHECK_RESULT(vkCreateSampler(VulkanContext::Get()->GetDevice(), &samplerInfo, nullptr, &m_Sampler));
    }
}