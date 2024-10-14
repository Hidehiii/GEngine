#include "GEpch.h"
#include "VulkanBuffer.h"
#include "VulkanContext.h"

namespace GEngine
{
    namespace Utils
    {
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
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

        void CreateBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
            VkBufferCreateInfo      bufferInfo{};
            bufferInfo.sType        = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size         = size;
            bufferInfo.usage        = usage;
            bufferInfo.sharingMode  = VK_SHARING_MODE_EXCLUSIVE;

            VK_CHECK_RESULT(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer));

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

            VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory));

            vkBindBufferMemory(device, buffer, bufferMemory, 0);
        }
    }


    VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size)
    {
        Utils::CreateBuffer(VulkanContext::GetDevice(), size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_VertexBuffer, m_VertexBufferMemory);
    }
    VulkanVertexBuffer::VulkanVertexBuffer(float* vertices, uint32_t size)
    {
        Utils::CreateBuffer(VulkanContext::GetDevice(), size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_VertexBuffer, m_VertexBufferMemory);
        SetData(vertices, size);
    }
    VulkanVertexBuffer::~VulkanVertexBuffer()
    {
        vkDestroyBuffer(VulkanContext::GetDevice(), m_VertexBuffer, nullptr);
        vkFreeMemory(VulkanContext::GetDevice(), m_VertexBufferMemory, nullptr);
    }
    void VulkanVertexBuffer::SetData(const void* data, uint32_t size)
    {
		void* mappedData;
		vkMapMemory(VulkanContext::GetDevice(), m_VertexBufferMemory, 0, size, 0, &mappedData);
		memcpy(mappedData, data, size);
		vkUnmapMemory(VulkanContext::GetDevice(), m_VertexBufferMemory);
    }
    void VulkanVertexBuffer::Bind() const
    {
        GE_CORE_ASSERT(VulkanContext::GetCurrentCommandBuffer(), "There is no commandbuffer be using");
        vkCmdBindVertexBuffers(VulkanContext::GetCurrentCommandBuffer(), 0, 1, &m_VertexBuffer, 0);
    }
    void VulkanVertexBuffer::Unbind() const
    {
    }
    
    VulkanIndexBuffer::VulkanIndexBuffer(uint32_t* indices, uint32_t count)
    {
		m_Count = count;
		VkDeviceSize bufferSize = sizeof(uint32_t) * count;
		Utils::CreateBuffer(VulkanContext::GetDevice(), bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_IndexBuffer, m_IndexBufferMemory);

		void* data;
		vkMapMemory(VulkanContext::GetDevice(), m_IndexBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices, (size_t)bufferSize);
		vkUnmapMemory(VulkanContext::GetDevice(), m_IndexBufferMemory);
    }

    VulkanIndexBuffer::~VulkanIndexBuffer()
    {
        vkDestroyBuffer(VulkanContext::GetDevice(), m_IndexBuffer, nullptr);
		vkFreeMemory(VulkanContext::GetDevice(), m_IndexBufferMemory, nullptr);
    }

    void VulkanIndexBuffer::Bind() const
    {
		GE_CORE_ASSERT(VulkanContext::GetCurrentCommandBuffer(), "There is no commandbuffer be using");
		vkCmdBindIndexBuffer(VulkanContext::GetCurrentCommandBuffer(), m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
    }

    void VulkanIndexBuffer::Unbind() const
    {
    }

}