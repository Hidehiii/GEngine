#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/RenderBuffer.h"

#include <vulkan/vulkan.h>

namespace GEngine
{
	class GENGINE_API VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(uint32_t size);
		VulkanVertexBuffer(float* vertices, uint32_t size);
		virtual ~VulkanVertexBuffer();

		virtual void SetData(const void* data, uint32_t size) override;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
		virtual const BufferLayout& GetLayout() const override { return m_Layout; }
	private:
		VkBuffer		m_VertexBuffer;
		VkDeviceMemory	m_VertexBufferMemory;
		BufferLayout	m_Layout;
		uint32_t		m_Offset = 0;
	};

	class GENGINE_API VulkanIndexBuffer : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(uint32_t* indices, uint32_t count);
		virtual ~VulkanIndexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual uint32_t GetCount() const override { return m_Count; };
	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
		VkBuffer		m_IndexBuffer;
		VkDeviceMemory	m_IndexBufferMemory;
	};


	
}



