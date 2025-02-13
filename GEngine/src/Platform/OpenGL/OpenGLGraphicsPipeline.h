#pragma once
#include "GEngine/Renderer/GraphicsPipeline.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include "Platform/OpenGL/OpenGLMaterial.h"

#include "Platform/OpenGL/OpenGLBuffer.h"

namespace GEngine
{
	class GENGINE_API OpenGLGraphicsPipeline : public GraphicsPipeline
	{
	public:
		OpenGLGraphicsPipeline(const Ref<Material>& material, const Ref<VertexBuffer>& vertexBuffer);
		virtual ~OpenGLGraphicsPipeline() override;
		virtual void Render(uint32_t instanceCount = 1, uint32_t indexCount = 0) override;
		virtual void Render(CommandBuffer* cmdBuffer, const Ref<RenderPass>& renderPass, uint32_t instanceCount = 1, uint32_t indexCount = 0) override;

		virtual Ref<VertexBuffer>	GetVertexBuffer() override { return std::static_pointer_cast<VertexBuffer>(m_VertexBuffer); }
		virtual void				SetVertexBuffer(Ref<VertexBuffer>& buffer) override;

		virtual Ref<Material>	GetMaterial() override { return std::static_pointer_cast<Material>(m_Material); }
		virtual void			SetMaterial(Ref<Material>& material) override;
	private:
		Ref<OpenGLMaterial>			m_Material;
		Ref<OpenGLVertexBuffer>		m_VertexBuffer;
	};
}


