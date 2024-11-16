#include "GEpch.h"
#include "VulkanBuffer.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"


namespace GEngine
{
    VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size, VertexTopology type)
    {
        m_TopologyType = type;
        Utils::CreateBuffer(VulkanContext::Get()->GetPhysicalDevice(), 
                            VulkanContext::Get()->GetDevice(),
                            size, 
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                            m_VertexBuffer, 
                            m_VertexBufferMemory);
    }
    VulkanVertexBuffer::VulkanVertexBuffer(float* vertices, uint32_t size, VertexTopology type)
    {
        m_TopologyType = type;
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
        if(m_IndexBuffer)
            m_IndexBuffer->Bind();
    }
    void VulkanVertexBuffer::Unbind() const
    {
    }

    void VulkanVertexBuffer::SetLayout(const BufferLayout& layout)
    {
        m_Layout                            = layout;
        VkVertexInputBindingDescription		bindingDescription{};
        bindingDescription.binding          = 0;
        bindingDescription.stride           = m_Layout.GetStride();
        bindingDescription.inputRate        = VK_VERTEX_INPUT_RATE_VERTEX;

        uint32_t index = 0;
        for (auto& element : m_Layout)
        {
            VkVertexInputAttributeDescription		attributeDescription{};
            attributeDescription.binding            = 0;
            attributeDescription.location           = index;
            attributeDescription.offset             = element.Offset;
            switch (element.Type)
            {
            case ShaderDataType::float1:
                attributeDescription.format = VK_FORMAT_R32_SFLOAT;
                break;
            case ShaderDataType::float2:
                attributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
                break;
            case ShaderDataType::float3:
                attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
                break;
            case ShaderDataType::float4:
                attributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                break;
            case ShaderDataType::int1:
                attributeDescription.format = VK_FORMAT_R32_SINT;
                break;
            case ShaderDataType::int2:
                attributeDescription.format = VK_FORMAT_R32G32_SINT;
                break;
            case ShaderDataType::int3:
                attributeDescription.format = VK_FORMAT_R32G32B32_SINT;
                break;
            case ShaderDataType::int4:
                attributeDescription.format = VK_FORMAT_R32G32B32A32_SINT;
                break;
            default:
                GE_CORE_ASSERT(false, "Unkonw Shader Date Type");
                break;
            }
            m_VertexInputAttributeDescriptions.push_back(attributeDescription);
            index++;
        }

        m_VertexInputBindingDescription = bindingDescription;
    }

    void VulkanVertexBuffer::SetIndexBuffer(const Ref<GEngine::IndexBuffer>& indexBuffer)
    {
        m_IndexBuffer = indexBuffer;
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