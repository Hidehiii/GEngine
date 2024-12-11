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
            m_Format = RenderImage2DFormat::RGBA8F;
        }
        else
        {
            m_Format = RenderImage2DFormat::RGB8F;
        }
        
        m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;
		Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
			                    VulkanContext::Get()->GetDevice(),
			                m_Width,
			                m_Height,
                            Utils::RenderImage2DFormatToVulkanFormat(m_Format),
			                VK_IMAGE_TILING_OPTIMAL,
			                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                            VK_SAMPLE_COUNT_1_BIT,
                            1,
                            0,
                            m_MipLevels,
			                m_Image,
			                m_ImageMemory);
        Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), m_Image, Utils::RenderImage2DFormatToVulkanFormat(m_Format), VK_IMAGE_VIEW_TYPE_2D, 1, VK_IMAGE_ASPECT_COLOR_BIT, m_MipLevels, m_ImageView);
        SetData(data, m_Width * m_Height * channels);

        stbi_image_free(data);
        
        CreateSampler();
    }
    VulkanTexture2D::VulkanTexture2D(uint32_t width, uint32_t height, RenderImage2DFormat format)
    {
        m_Height    = height;
        m_Width     = width;
        m_Format = format;

        m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;
		Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
			                VulkanContext::Get()->GetDevice(),
			                m_Width,
			                m_Height,
                            Utils::RenderImage2DFormatToVulkanFormat(m_Format),
			                VK_IMAGE_TILING_OPTIMAL,
                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                            VK_SAMPLE_COUNT_1_BIT,
                            1,
                            0,
                            m_MipLevels,
			                m_Image,
			                m_ImageMemory);
		Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), m_Image, Utils::RenderImage2DFormatToVulkanFormat(m_Format), VK_IMAGE_VIEW_TYPE_2D, 1, VK_IMAGE_ASPECT_COLOR_BIT, m_MipLevels, m_ImageView);

        CreateSampler();
    }
    VulkanTexture2D::VulkanTexture2D(uint32_t width, uint32_t height, void* data, uint32_t size, RenderImage2DFormat format)
    {
		m_Height = height;
		m_Width = width;
        m_Format = format;

        m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;
		Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
			VulkanContext::Get()->GetDevice(),
			m_Width,
			m_Height,
            Utils::RenderImage2DFormatToVulkanFormat(m_Format),
			VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_SAMPLE_COUNT_1_BIT,
            1,
            0,
            m_MipLevels,
			m_Image,
			m_ImageMemory);
		Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), m_Image, Utils::RenderImage2DFormatToVulkanFormat(m_Format), VK_IMAGE_VIEW_TYPE_2D, 1, VK_IMAGE_ASPECT_COLOR_BIT, m_MipLevels, m_ImageView);

		CreateSampler();
        SetData(data, size);
    }
    VulkanTexture2D::VulkanTexture2D(VkFormat format, VkImage image, VkImageView imageView, VkDeviceMemory imageMemory, VkImageLayout layout, VkFlags aspectFlag, bool isMultiSample)
    {
        m_Format = Utils::VulkanFormatToRenderImage2DFormat(format);
        m_Image         = image;
        m_ImageView     = imageView;
        m_ImageLayout   = layout;
        m_ImageMemory   = imageMemory;
        m_AspectFlag    = aspectFlag;
        m_MultiSample   = isMultiSample;
        CreateSampler();
    }
    VulkanTexture2D::~VulkanTexture2D()
    {
        vkDeviceWaitIdle(VulkanContext::Get()->GetDevice());
		vkDestroyImageView(VulkanContext::Get()->GetDevice(), m_ImageView, nullptr);
		vkDestroyImage(VulkanContext::Get()->GetDevice(), m_Image, nullptr);
		vkFreeMemory(VulkanContext::Get()->GetDevice(), m_ImageMemory, nullptr);
        vkDestroySampler(VulkanContext::Get()->GetDevice(), m_Sampler, nullptr);
    }

    void VulkanTexture2D::SetData(const void* data, uint32_t size)
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
        Utils::CopyBufferToImage(buffer, m_Image, m_Width, m_Height, 0, 0, m_AspectFlag);
		Utils::GenerateMipmap(m_Image, m_Width, m_Height, m_MipLevels, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1);
        m_ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        
        vkDestroyBuffer(VulkanContext::Get()->GetDevice(), buffer, nullptr);
        vkFreeMemory(VulkanContext::Get()->GetDevice(), memory, nullptr);
    }
    void VulkanTexture2D::Bind(const uint32_t slot)
    {
        GE_CORE_ASSERT(m_MultiSample == false, "Can not bind a multisample texture!");
        SetImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		m_ImageInfo.imageLayout       = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_ImageInfo.imageView         = m_ImageView;
		m_ImageInfo.sampler           = m_Sampler;
    }

    void VulkanTexture2D::SetImageLayout(VkImageLayout newLayout)
    {
        Utils::TransitionImageLayout(m_Image, Utils::RenderImage2DFormatToVulkanFormat(m_Format), m_ImageLayout, newLayout, 1, m_AspectFlag, m_MipLevels);
        m_ImageLayout = newLayout;
    }
    void VulkanTexture2D::CreateSampler()
    {
		VkSamplerCreateInfo             samplerInfo{};
		samplerInfo.sType               = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter           = VK_FILTER_NEAREST;
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
		samplerInfo.maxLod                      = VK_LOD_CLAMP_NONE;

        VK_CHECK_RESULT(vkCreateSampler(VulkanContext::Get()->GetDevice(), &samplerInfo, nullptr, &m_Sampler));
    }
}