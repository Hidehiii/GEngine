#include "GEpch.h"
#include "VulkanTexture2D.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "stb_image.h"

namespace GEngine
{
    VulkanTexture2D::VulkanTexture2D(const std::string& path)
    {
        m_Path      = path;
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
    }
    void VulkanTexture2D::Bind(const uint32_t slot) const
    {
    }
}