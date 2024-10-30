#pragma once

#include "GEngine/Renderer/VertexArray.h"

namespace GEngine
{
	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;

		virtual const Ref<VertexBuffer>& GetVertexBuffer() const override { return m_VertexBuffer; }
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }

	private:
		uint32_t								m_RendererID;
		uint32_t								m_VertexBufferIndex = 0;
		Ref<VertexBuffer>						m_VertexBuffer;
		Ref<IndexBuffer>						m_IndexBuffer;
	};
}
