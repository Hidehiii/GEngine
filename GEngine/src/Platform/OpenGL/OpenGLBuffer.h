#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/RenderBuffer.h"

namespace GEngine
{
	class GENGINE_API OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(uint32_t size, uint32_t sizeInstance = 0, VertexTopology type = VertexTopology::Triangle);
		OpenGLVertexBuffer(float* vertices, uint32_t size, uint32_t sizeInstance = 0, VertexTopology type = VertexTopology::Triangle);
		virtual ~OpenGLVertexBuffer();

		virtual void Bind() const override;

		virtual void SetData(const void* data, uint32_t size) override;
		virtual void SetDataInstance(const void* data, uint32_t size) override;
		virtual void SetLayout(const BufferLayout& layout) override;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;
		
		virtual const BufferLayout&		GetLayout() const override { return m_Layout; }
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }
		virtual VertexTopology			GetVertexTopologyType() override { return m_TopologyType; }
		virtual bool					IsInstanceRendering() override { return m_InstanceRendering; }
	private:
		uint32_t								m_VertexBuffer;
		uint32_t								m_InstanceBuffer;
		uint32_t								m_VertexArray;
		BufferLayout							m_Layout;
		Ref<IndexBuffer>						m_IndexBuffer;
		VertexTopology							m_TopologyType;
		bool									m_InstanceRendering = false;
	};


	class GENGINE_API OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(const uint32_t* indices, uint32_t count);
		virtual ~OpenGLIndexBuffer();

		virtual void Bind() const override;
		virtual uint32_t GetCount() const override { return m_Count;  };
	private:
		uint32_t m_Buffer;
		uint32_t m_Count;
	};
}

