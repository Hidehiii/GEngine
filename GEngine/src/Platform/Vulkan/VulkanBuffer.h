#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/RenderBuffer.h"

#include <vulkan/vulkan.h>

namespace GEngine
{
	class GENGINE_API VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(uint32_t size, uint32_t sizeInstance = 0, VertexTopology type = VertexTopology::Triangle);
		VulkanVertexBuffer(float* vertices, uint32_t size, uint32_t sizeInstance = 0, VertexTopology type = VertexTopology::Triangle);
		virtual ~VulkanVertexBuffer();

		virtual void SetData(const void* data, uint32_t size) override;
		virtual void SetDataInstance(const void* data, uint32_t size) override;
		virtual void Bind() const override;


		virtual void SetLayout(const BufferLayout& layout) override;
		virtual const BufferLayout& GetLayout() const override { return m_Layout; }
		virtual void SetIndexBuffer(const Ref<GEngine::IndexBuffer>& indexBuffer) override;
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }
		virtual VertexTopology GetVertexTopologyType() override { return m_TopologyType; }
		virtual bool IsInstanceRendering() override { return m_InstanceRendering; }
		const std::vector<VkVertexInputBindingDescription>& GetVertexInputBindingDescription() const { return m_VertexInputBindingDescription; }
		const std::vector<VkVertexInputAttributeDescription>& GetVertexInputAttributeDescriptions() const { return m_VertexInputAttributeDescriptions; }
	private:
		VertexTopology	m_TopologyType;
		VkBuffer		m_VertexBuffer;
		VkBuffer		m_InstanceBuffer;
		VkDeviceMemory	m_VertexBufferMemory;
		VkDeviceMemory	m_InstanceBufferMemory;
		BufferLayout	m_Layout;
		uint32_t		m_Offset = 0;
		Ref<IndexBuffer>						m_IndexBuffer = nullptr;
		std::vector<VkVertexInputBindingDescription>	m_VertexInputBindingDescription;
		std::vector<VkVertexInputAttributeDescription>	m_VertexInputAttributeDescriptions;
		bool									m_InstanceRendering = false;
	};

	class GENGINE_API VulkanIndexBuffer : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(const uint32_t* indices, uint32_t count);
		virtual ~VulkanIndexBuffer();

		virtual void Bind() const override;
		virtual uint32_t GetCount() const override { return m_Count; };
	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
		VkBuffer		m_IndexBuffer;
		VkDeviceMemory	m_IndexBufferMemory;
	};


	
}



