#include "GEpch.h"
#include "VulkanBuffer.h"
#include "VulkanContext.h"

namespace GEngine
{
    VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size)
    {
        CreateBuffer(size);
    }
    VulkanVertexBuffer::VulkanVertexBuffer(float* vertices, uint32_t size)
    {
        CreateBuffer(size);
    }
    VulkanVertexBuffer::~VulkanVertexBuffer()
    {
        vkDestroyBuffer(VulkanContext::GetDevice(), m_VertexBuffer, nullptr);
        vkFreeMemory(VulkanContext::GetDevice(), m_VertexBufferMemory, nullptr);
    }
    void VulkanVertexBuffer::SetData(const void* data, uint32_t size)
    {
    }
    void VulkanVertexBuffer::Bind() const
    {
    }
    void VulkanVertexBuffer::Unbind() const
    {
    }
    void VulkanVertexBuffer::CreateBuffer(uint32_t size)
    {
        VkBufferCreateInfo      bufferInfo{};
        bufferInfo.sType        = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size         = size;
        bufferInfo.usage        = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferInfo.sharingMode  = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(VulkanContext::GetDevice(), &bufferInfo, nullptr, &m_VertexBuffer) != VK_SUCCESS) {
            GE_CORE_ERROR("failed to create vertex buffer!");
        }

        VkMemoryRequirements    memRequirements;
        vkGetBufferMemoryRequirements(VulkanContext::GetDevice(), m_VertexBuffer, &memRequirements);

        VkMemoryAllocateInfo        allocInfo{};
        allocInfo.sType             = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize    = memRequirements.size;
        allocInfo.memoryTypeIndex   = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vkAllocateMemory(VulkanContext::GetDevice(), &allocInfo, nullptr, &m_VertexBufferMemory) != VK_SUCCESS) {
            GE_CORE_ERROR("failed to allocate vertex buffer memory!");
        }
    }
    uint32_t VulkanVertexBuffer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties    memProperties;
        vkGetPhysicalDeviceMemoryProperties(VulkanContext::GetPhysicalDevice(), &memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        GE_CORE_ERROR("failed to find suitable memory type!");
        return 0;
    }
}