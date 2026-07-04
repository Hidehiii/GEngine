#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/VertexBuffer.h"

namespace GEngine
{

	class OpenGLIndexBuffer;

	class GENGINE_API OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(uint32_t size, uint32_t sizeInstance = 0, VertexTopology type = VERTEX_TOPOLOGY_TRIANGLE);
		OpenGLVertexBuffer(float* vertices, uint32_t size, uint32_t sizeInstance = 0, VertexTopology type = VERTEX_TOPOLOGY_TRIANGLE);
		virtual ~OpenGLVertexBuffer();

		virtual void SetVertexData(const void* data, uint32_t size) override;
		virtual void SetInstanceData(const void* data, uint32_t size) override;
		virtual void SetLayout(const ShaderInputBufferLayout& layout) override;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;
		
		virtual const ShaderInputBufferLayout&		GetLayout() const override { return m_Layout; }
		virtual const Ref<IndexBuffer>&				GetIndexBuffer() const override { return std::dynamic_pointer_cast<IndexBuffer>(m_IndexBuffer); }
		virtual VertexTopology						GetVertexTopologyType() override { return m_TopologyType; }
		virtual bool								IsInstanceRendering() override { return m_InstanceRendering; }

		virtual uint32_t GetIndexCount() const override;
	protected:
		virtual void Bind(CommandBuffer* cmd) const override;
	private:
		uint32_t								m_VertexBuffer = 0;
		uint32_t								m_InstanceBuffer = 0;
		uint32_t								m_VertexArray;
		Ref<OpenGLIndexBuffer>					m_IndexBuffer = nullptr;

		friend class OpenGLGraphicsPipeline;
	};


	class GENGINE_API OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(const uint32_t* indices, uint32_t count);
		virtual ~OpenGLIndexBuffer();

	protected:
		virtual void Bind(CommandBuffer* cmd) const override;
	private:
		uint32_t m_Buffer;
		

		friend class OpenGLVertexBuffer;
	};
}

