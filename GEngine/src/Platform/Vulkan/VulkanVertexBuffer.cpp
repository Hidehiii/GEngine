#include "GEpch.h"
#include "Platform/Vulkan/VulkanVertexBuffer.h"
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
    VulkanVertexBuffer::VulkanVertexBuffer(const void* vertices, uint32_t size, uint32_t sizeInstance, VertexTopology type)
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
        auto* context = VulkanContext::Get();
        if (context != nullptr && context->GetDevice() != VK_NULL_HANDLE)
        {
			const VkBuffer vertexBuffer = m_VertexBuffer;
			const VkDeviceMemory vertexMemory = m_VertexBufferMemory;
			const VkBuffer instanceBuffer = m_InstanceBuffer;
			const VkDeviceMemory instanceMemory = m_InstanceBufferMemory;
			context->RetireResource([vertexBuffer, vertexMemory, instanceBuffer, instanceMemory](VkDevice device)
			{
				if (vertexBuffer != VK_NULL_HANDLE)
					vkDestroyBuffer(device, vertexBuffer, nullptr);
				if (vertexMemory != VK_NULL_HANDLE)
					vkFreeMemory(device, vertexMemory, nullptr);
				if (instanceBuffer != VK_NULL_HANDLE)
					vkDestroyBuffer(device, instanceBuffer, nullptr);
				if (instanceMemory != VK_NULL_HANDLE)
					vkFreeMemory(device, instanceMemory, nullptr);
			});
        }
		m_VertexBuffer = VK_NULL_HANDLE;
		m_VertexBufferMemory = VK_NULL_HANDLE;
		m_InstanceBuffer = VK_NULL_HANDLE;
		m_InstanceBufferMemory = VK_NULL_HANDLE;
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

    Ref<IndexBuffer> VulkanVertexBuffer::GetIndexBuffer() const
    {
        return m_IndexBuffer;
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
        auto* context = VulkanContext::Get();
        if (context != nullptr && context->GetDevice() != VK_NULL_HANDLE)
        {
			const VkBuffer indexBuffer = m_IndexBuffer;
			const VkDeviceMemory indexMemory = m_IndexBufferMemory;
			context->RetireResource([indexBuffer, indexMemory](VkDevice device)
			{
				if (indexBuffer != VK_NULL_HANDLE)
					vkDestroyBuffer(device, indexBuffer, nullptr);
				if (indexMemory != VK_NULL_HANDLE)
					vkFreeMemory(device, indexMemory, nullptr);
			});
        }
		m_IndexBuffer = VK_NULL_HANDLE;
		m_IndexBufferMemory = VK_NULL_HANDLE;
    }
    void VulkanIndexBuffer::Bind(CommandBuffer* cmd) const
    {
        vkCmdBindIndexBuffer(static_cast<VulkanCommandBuffer*>(cmd)->GetCommandBuffer(), m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
    }
}
