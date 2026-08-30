#pragma once
#include "GEngine/Graphics/GraphicsPipeline.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include "Platform/OpenGL/OpenGLMaterial.h"

#include "Platform/OpenGL/OpenGLVertexBuffer.h"

namespace GEngine
{
	class GENGINE_API OpenGLGraphicsPipeline : public GraphicsPipeline
	{
	public:
		OpenGLGraphicsPipeline(const Ref<Material>& material, const Ref<VertexBuffer>& vertexBuffer);
		virtual ~OpenGLGraphicsPipeline() override;
		

		virtual Ref<VertexBuffer>	GetVertexBuffer() override { return std::static_pointer_cast<VertexBuffer>(m_VertexBuffer); }
		virtual void				SetVertexBuffer(Ref<VertexBuffer>& buffer) override;

		virtual Ref<Material>	GetMaterial() override { return std::static_pointer_cast<Material>(m_Material); }
		virtual void			SetMaterial(Ref<Material>& material) override;

		bool operator==(const OpenGLGraphicsPipeline& other) const
		{
			return m_Material == other.m_Material && m_VertexBuffer == other.m_VertexBuffer;
		}
	protected:
		virtual void Render(CommandBuffer* cmdBuffer, const Ref<FrameBuffer>& frameBuffer, uint32_t pass, uint32_t instanceCount = 1, uint32_t indexCount = 0) override;
	private:
		Ref<OpenGLMaterial>			m_Material;
		Ref<OpenGLVertexBuffer>		m_VertexBuffer;

		friend class OpenGLCommandBuffer;
	};
}


