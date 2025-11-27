#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsBuffer.h"

#include <vulkan/vulkan.h>

namespace GEngine
{


	class VulkanIndexBuffer;

	class GENGINE_API VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(uint32_t size, uint32_t sizeInstance = 0, VertexTopology type = VERTEX_TOPOLOGY_TRIANGLE);
		VulkanVertexBuffer(float* vertices, uint32_t size, uint32_t sizeInstance = 0, VertexTopology type = VERTEX_TOPOLOGY_TRIANGLE);
		virtual ~VulkanVertexBuffer();

		virtual void SetData(const void* data, uint32_t size) override;
		virtual void SetDataInstance(const void* data, uint32_t size) override;
		virtual void SetLayout(const ShaderInputBufferLayout& layout) override;
		virtual void SetIndexBuffer(const Ref<GEngine::IndexBuffer>& indexBuffer) override;

		virtual const ShaderInputBufferLayout&		GetLayout() const override { return m_Layout; }
		virtual const Ref<IndexBuffer>&				GetIndexBuffer() const override { return std::dynamic_pointer_cast<IndexBuffer>(m_IndexBuffer); }
		virtual VertexTopology						GetVertexTopologyType() override { return m_TopologyType; }
		virtual bool								IsInstanceRendering() override { return m_InstanceRendering; }

		const std::vector<VkVertexInputBindingDescription>&		GetVertexInputBindingDescription() const { return m_VertexInputBindingDescription; }
		const std::vector<VkVertexInputAttributeDescription>&	GetVertexInputAttributeDescriptions() const { return m_VertexInputAttributeDescriptions; }

	protected:
		virtual void Bind(CommandBuffer* cmd) const override;
	private:
		
		VkBuffer		m_VertexBuffer;
		VkBuffer		m_InstanceBuffer;
		VkDeviceMemory	m_VertexBufferMemory;
		VkDeviceMemory	m_InstanceBufferMemory;
		
		uint32_t		m_Offset = 0;
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
		

	protected:
		virtual void Bind(CommandBuffer* cmd) const override;
	private:
		
		VkBuffer		m_IndexBuffer;
		VkDeviceMemory	m_IndexBufferMemory;

		friend class VulkanVertexBuffer;
	};


	
}



