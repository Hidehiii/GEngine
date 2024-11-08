#include "GEpch.h"
#include "VulkanBuffer.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"


namespace GEngine
{
    VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size)
    {
        Utils::CreateBuffer(VulkanContext::Get()->GetPhysicalDevice(), 
                            VulkanContext::Get()->GetDevice(),
                            size, 
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                            m_VertexBuffer, 
                            m_VertexBufferMemory);
    }
    VulkanVertexBuffer::VulkanVertexBuffer(float* vertices, uint32_t size)
    {
        Utils::CreateBuffer(VulkanContext::Get()->GetPhysicalDevice(),
                            VulkanContext::Get()->GetDevice(),
                            size, 
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                            m_VertexBuffer, 
                            m_VertexBufferMemory);
        SetData(vertices, size);
    }
    VulkanVertexBuffer::~VulkanVertexBuffer()
    {
        vkDestroyBuffer(VulkanContext::Get()->GetDevice(), m_VertexBuffer, nullptr);
        vkFreeMemory(VulkanContext::Get()->GetDevice(), m_VertexBufferMemory, nullptr);
    }
    void VulkanVertexBuffer::SetData(const void* data, uint32_t size)
    {
		void* mappedData;
		vkMapMemory(VulkanContext::Get()->GetDevice(), m_VertexBufferMemory, 0, size, 0, &mappedData);
		memcpy(mappedData, data, size);
		vkUnmapMemory(VulkanContext::Get()->GetDevice(), m_VertexBufferMemory);
    }
    void VulkanVertexBuffer::Bind() const
    {
        GE_CORE_ASSERT(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), "There is no commandbuffer be using");
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), 0, 1, &m_VertexBuffer, offsets);
    }
    void VulkanVertexBuffer::Unbind() const
    {
    }
    
    VulkanIndexBuffer::VulkanIndexBuffer(uint32_t* indices, uint32_t count)
    {
		m_Count = count;
		VkDeviceSize bufferSize = sizeof(uint32_t) * count;
		Utils::CreateBuffer(VulkanContext::Get()->GetPhysicalDevice(), VulkanContext::Get()->GetDevice(), bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_IndexBuffer, m_IndexBufferMemory);

		void* data;
		vkMapMemory(VulkanContext::Get()->GetDevice(), m_IndexBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices, (size_t)bufferSize);
		vkUnmapMemory(VulkanContext::Get()->GetDevice(), m_IndexBufferMemory);
    }

    VulkanIndexBuffer::~VulkanIndexBuffer()
    {
        vkDestroyBuffer(VulkanContext::Get()->GetDevice(), m_IndexBuffer, nullptr);
		vkFreeMemory(VulkanContext::Get()->GetDevice(), m_IndexBufferMemory, nullptr);
    }

    void VulkanIndexBuffer::Bind() const
    {
		GE_CORE_ASSERT(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), "There is no commandbuffer be using");
		vkCmdBindIndexBuffer(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
    }

    void VulkanIndexBuffer::Unbind() const
    {
    }

}