#pragma once

#include "GEngine/Renderer/VertexArray.h"

namespace GEngine
{
	class OpenGLVertexArray : public GEngine::VertexArray
	{
	public:
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AddVertexBuffer(const Ref<GEngine::VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const Ref<GEngine::IndexBuffer>& indexBuffer) override;

		virtual const std::vector<Ref<GEngine::VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
		virtual const Ref<GEngine::IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }

	private:
		uint32_t m_RendererID;
		uint32_t m_VertexBufferIndex = 0;
		std::vector<Ref<GEngine::VertexBuffer>> m_VertexBuffers;
		Ref<GEngine::IndexBuffer> m_IndexBuffer;
	};
}
