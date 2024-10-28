#include "GEpch.h"
#include "VulkanTexture2D.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "stb_image.h"

namespace GEngine
{
    VulkanTexture2D::VulkanTexture2D(const std::string& path)
    {
        m_Path      = path;
		int			width, height, channels;
		stbi_uc* data;
		stbi_set_flip_vertically_on_load(1);
		{
			GE_PROFILE_SCOPE("stbi_load - OpenGLTexture2D::OpenGLTexture2D(const std::string& path)")
				data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		}
		GE_CORE_ASSERT(data, "Failed to load image!");
		m_Width     = width;
		m_Height    = height;
        if (channels == 4)
        {
            m_DataFormat = VK_FORMAT_R8G8B8A8_SNORM;
        }
        else
        {
            m_DataFormat = VK_FORMAT_R8G8B8_SNORM;
        }
        Utils::CreateBuffer(VulkanContext::GetPhysicalDevice(),
			                VulkanContext::GetDevice(),
			                static_cast<uint32_t>(width * height * channels),
			                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			                m_StagingBuffer,
			                m_StagingBufferMemory);

        SetData(data, m_Width * m_Height * channels);
        stbi_image_free(data);
    }
    VulkanTexture2D::VulkanTexture2D(uint32_t width, uint32_t height)
    {
        m_Height    = height;
        m_Width     = width;
    }
    VulkanTexture2D::~VulkanTexture2D()
    {
    }
    void VulkanTexture2D::SetData(void* data, uint32_t size)
    {
        uint32_t bpp = m_DataFormat == VK_FORMAT_R8G8B8A8_SNORM ? 4 : 3;
        GE_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture!");

        void* tempData;
        vkMapMemory(VulkanContext::GetDevice(), m_StagingBufferMemory, 0, size, 0, &tempData);
        memcpy(tempData, data, static_cast<size_t>(size));
        vkUnmapMemory(VulkanContext::GetDevice(), m_StagingBufferMemory);
    }
    void VulkanTexture2D::Bind(const uint32_t slot) const
    {
    }
}