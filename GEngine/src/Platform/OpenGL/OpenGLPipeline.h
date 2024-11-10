#pragma once
#include "GEngine/Renderer/Pipeline.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include "Platform/OpenGL/OpenGLMaterial.h"

#include "Platform/OpenGL/OpenGLBuffer.h"

namespace GEngine
{
	class GENGINE_API OpenGLPipeline : public Pipeline
	{
	public:
		OpenGLPipeline(const Ref<Material>& material, const Ref<VertexBuffer>& vertexBuffer);
		virtual ~OpenGLPipeline() override;
		virtual void Bind() override;
		virtual void Unbind() override;

		virtual Ref<VertexBuffer> GetVertexBuffer() override { return std::static_pointer_cast<VertexBuffer>(m_VertexBuffer); }
		virtual Ref<Material> GetMaterial() override { return std::static_pointer_cast<Material>(m_Material); }
	private:
		Ref<OpenGLMaterial>			m_Material;
		Ref<OpenGLVertexBuffer>		m_VertexBuffer;
	};
}


