#include "GEpch.h"
#include "VulkanVertexBuffer.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanUtils.h"


namespace GEngine
{
    VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size, uint32_t sizeInstance, VertexTopology type)
    {
        m_TopologyType = type;
		m_TotalSizeVertex = size;
        Utils::CreateBuffer(VulkanContext::Get()->GetPhysicalDevice(), 
                            VulkanContext::Get()->GetDevice(),
                            size, 
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                            m_VertexBuffer, 
                            m_VertexBufferMemory);
        if (sizeInstance > 0)
        {
			m_TotalSizeInstance = sizeInstance;
            Utils::CreateBuffer(VulkanContext::Get()->GetPhysicalDevice(),
                VulkanContext::Get()->GetDevice(),
                sizeInstance,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_InstanceBuffer,
                m_InstanceBufferMemory);
        }
    }
    VulkanVertexBuffer::VulkanVertexBuffer(float* vertices, uint32_t size, uint32_t sizeInstance, VertexTopology type)
    {
        m_TopologyType = type;
        m_TotalSizeVertex = size;
        Utils::CreateBuffer(VulkanContext::Get()->GetPhysicalDevice(),
                            VulkanContext::Get()->GetDevice(),
                            size, 
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                            m_VertexBuffer, 
                            m_VertexBufferMemory);
        if (sizeInstance > 0)
        {
			m_TotalSizeInstance = sizeInstance;
            Utils::CreateBuffer(VulkanContext::Get()->GetPhysicalDevice(),
                VulkanContext::Get()->GetDevice(),
                sizeInstance,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_InstanceBuffer,
                m_InstanceBufferMemory);
        }
        SetVertexData(vertices, size);
    }
    VulkanVertexBuffer::~VulkanVertexBuffer()
    {
        if (VulkanContext::Get()->GetDevice())
        {
			vkDestroyBuffer(VulkanContext::Get()->GetDevice(), m_VertexBuffer, nullptr);
			vkFreeMemory(VulkanContext::Get()->GetDevice(), m_VertexBufferMemory, nullptr);
            if (m_InstanceBuffer != nullptr)
            {
				vkDestroyBuffer(VulkanContext::Get()->GetDevice(), m_InstanceBuffer, nullptr);
				vkFreeMemory(VulkanContext::Get()->GetDevice(), m_InstanceBufferMemory, nullptr);
            }
        }
        
    }
    void VulkanVertexBuffer::SetVertexData(const void* data, uint32_t size)
    {
		void* mappedData;
		vkMapMemory(VulkanContext::Get()->GetDevice(), m_VertexBufferMemory, 0, size, 0, &mappedData);
		memcpy(mappedData, data, size);
		vkUnmapMemory(VulkanContext::Get()->GetDevice(), m_VertexBufferMemory);
    }
    void VulkanVertexBuffer::SetInstanceData(const void* data, uint32_t size)
    {
		GE_CORE_ASSERT(IsInstanceRendering() == true, "Instance rendering is not enabled for this vertex buffer.");
        void* mappedData;
        vkMapMemory(VulkanContext::Get()->GetDevice(), m_InstanceBufferMemory, 0, size, 0, &mappedData);
        memcpy(mappedData, data, size);
        vkUnmapMemory(VulkanContext::Get()->GetDevice(), m_InstanceBufferMemory);
    }

    void VulkanVertexBuffer::SetIndexBuffer(const Ref<GEngine::IndexBuffer>& indexBuffer)
    {
        m_IndexBuffer = std::static_pointer_cast<VulkanIndexBuffer>(indexBuffer);
    }


    uint32_t VulkanVertexBuffer::GetIndexCount() const
    {
        return m_IndexBuffer != nullptr ? m_IndexBuffer->GetCount() : 0;
    }


    void VulkanVertexBuffer::Bind(CommandBuffer* cmd) const
    {
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(static_cast<VulkanCommandBuffer*>(cmd)->GetCommandBuffer(), 0, 1, &m_VertexBuffer, offsets);
		if (m_InstanceBuffer)
			vkCmdBindVertexBuffers(static_cast<VulkanCommandBuffer*>(cmd)->GetCommandBuffer(), 1, 1, &m_InstanceBuffer, offsets);
		if (m_IndexBuffer)
			m_IndexBuffer->Bind(cmd);
    }

    void VulkanVertexBuffer::SetShaderAndInputLayout(const Ref<Shader>& shader, uint32_t pass)
    {
		GE_CORE_ASSERT(shader, "Shader is null!");
		m_Shader = shader;
		m_ShaderPass = pass;

        m_VertexInputBindingDescription.clear();
        m_VertexInputAttributeDescriptions.clear();

        VkVertexInputBindingDescription		bindingDescription{};
        bindingDescription.binding          = 0;
        bindingDescription.stride           = m_Shader->GetPassReflections().at(m_ShaderPass).VertexInputVertexStride;
        bindingDescription.inputRate        = VK_VERTEX_INPUT_RATE_VERTEX;
        m_VertexInputBindingDescription.push_back(bindingDescription);

        if (m_InstanceBuffer)
        {
            VkVertexInputBindingDescription		    instanceBindingDescription{};
            instanceBindingDescription.binding      = 1;
            instanceBindingDescription.stride       = m_Shader->GetPassReflections().at(m_ShaderPass).VertexInputInstanceStride;
            instanceBindingDescription.inputRate    = VK_VERTEX_INPUT_RATE_INSTANCE;
            m_VertexInputBindingDescription.push_back(instanceBindingDescription);
        }

        VkVertexInputAttributeDescription		attributeDescription{};

        for (auto& e : m_Shader->GetPassReflections().at(m_ShaderPass).VertexInputs)
        {
            attributeDescription.binding    = e.IsPerInstance ? 1 : 0;
            attributeDescription.location   = e.Location;
            attributeDescription.offset     = e.Offset;

			attributeDescription.format = Utils::ShaderInputDataToVulkanFormat(e.Type);
            m_VertexInputAttributeDescriptions.push_back(attributeDescription);
        }
    }
    
    VulkanIndexBuffer::VulkanIndexBuffer(const uint32_t* indices, uint32_t count)
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
        if (VulkanContext::Get()->GetDevice())
        {
			vkDestroyBuffer(VulkanContext::Get()->GetDevice(), m_IndexBuffer, nullptr);
			vkFreeMemory(VulkanContext::Get()->GetDevice(), m_IndexBufferMemory, nullptr);
        }
        
    }
    void VulkanIndexBuffer::Bind(CommandBuffer* cmd) const
    {
        vkCmdBindIndexBuffer(static_cast<VulkanCommandBuffer*>(cmd)->GetCommandBuffer(), m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
    }
}