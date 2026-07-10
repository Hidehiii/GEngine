#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/VertexBuffer.h"

#include <vulkan/vulkan.h>

namespace GEngine
{
	class VulkanIndexBuffer;

	class GENGINE_API VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(uint32_t size, uint32_t sizeInstance = 0, VertexTopology type = VERTEX_TOPOLOGY_TRIANGLE);
		VulkanVertexBuffer(const void* vertices, uint32_t size, uint32_t sizeInstance = 0, VertexTopology type = VERTEX_TOPOLOGY_TRIANGLE);
		virtual ~VulkanVertexBuffer();

		virtual void SetVertexData(const void* data, uint32_t size) override;
		virtual void SetInstanceData(const void* data, uint32_t size) override;
		virtual void SetIndexBuffer(const Ref<GEngine::IndexBuffer>& indexBuffer) override;


		virtual const Ref<IndexBuffer>&				GetIndexBuffer() const override { return std::dynamic_pointer_cast<IndexBuffer>(m_IndexBuffer); }
		virtual VertexTopology						GetVertexTopologyType() override { return m_TopologyType; }


		virtual uint32_t GetIndexCount() const override;

		const std::vector<VkVertexInputBindingDescription>&		GetVertexInputBindingDescription() const { return m_VertexInputBindingDescription; }
		const std::vector<VkVertexInputAttributeDescription>&	GetVertexInputAttributeDescriptions() const { return m_VertexInputAttributeDescriptions; }

		bool operator==(const VulkanVertexBuffer& other) const
		{
			return m_VertexBuffer == other.m_VertexBuffer && m_InstanceBuffer == other.m_InstanceBuffer && m_IndexBuffer == other.m_IndexBuffer;
		}
	protected:
		virtual void Bind(CommandBuffer* cmd) const override;

		virtual void SetShaderAndInputLayout(const Ref<Shader>& shader, uint32_t pass) override;
	private:
		
		VkBuffer		m_VertexBuffer = nullptr;
		VkBuffer		m_InstanceBuffer = nullptr;
		VkDeviceMemory	m_VertexBufferMemory;
		VkDeviceMemory	m_InstanceBufferMemory;
		
		std::vector<VkVertexInputBindingDescription>	m_VertexInputBindingDescription;
		std::vector<VkVertexInputAttributeDescription>	m_VertexInputAttributeDescriptions;

		Ref<VulkanIndexBuffer>					m_IndexBuffer = nullptr;

		friend class VulkanGraphicsPipeline;
	};

	class GENGINE_API VulkanIndexBuffer : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(const uint32_t* indices, uint32_t count);
		virtual ~VulkanIndexBuffer();
		
		bool operator==(const VulkanIndexBuffer& other) const
		{
			return m_IndexBuffer == other.m_IndexBuffer;
		}
	protected:
		virtual void Bind(CommandBuffer* cmd) const override;
	private:
		
		VkBuffer		m_IndexBuffer;
		VkDeviceMemory	m_IndexBufferMemory;

		friend class VulkanVertexBuffer;
	};


	
}



