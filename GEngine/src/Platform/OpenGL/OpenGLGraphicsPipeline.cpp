#include "GEpch.h"
#include "OpenGLGraphicsPipeline.h"
#include "GEngine/Graphics/Graphics.h"
#include <glad/glad.h>

namespace GEngine
{
    OpenGLGraphicsPipeline::OpenGLGraphicsPipeline(const Ref<Material>& material, const Ref<VertexBuffer>& vertexBuffer)
    {
        m_Material      = std::dynamic_pointer_cast<OpenGLMaterial>(material);
        m_VertexBuffer  = std::dynamic_pointer_cast<OpenGLVertexBuffer>(vertexBuffer);
    }
    OpenGLGraphicsPipeline::~OpenGLGraphicsPipeline()
    {
    }
	void OpenGLGraphicsPipeline::Render(CommandBuffer* cmdBuffer, const Ref<FrameBuffer>& frameBuffer, int pass, uint32_t instanceCount, uint32_t indexCount)
	{
		m_Material->Update(cmdBuffer, pass);
		m_VertexBuffer->Bind(cmdBuffer);
		indexCount = indexCount > 0 ? indexCount : m_VertexBuffer->GetIndexCount();
		instanceCount = instanceCount > 0 ? instanceCount : m_VertexBuffer->GetInstanceCount();
		GLint mode;
		switch (m_VertexBuffer->GetVertexTopologyType())
		{
		case VERTEX_TOPOLOGY_TRIANGLE:
		{
			mode = GL_TRIANGLES;
			break;
		}
		case VERTEX_TOPOLOGY_LINE:
		{
			mode = GL_LINES;
			break;
		}
		case VERTEX_TOPOLOGY_POINT:
		{
			mode = GL_POINTS;
			break;
		}
		default:
			GE_CORE_ASSERT(false, "Unknow type");
		}
		if (indexCount == 0)
		{
			glDrawArraysInstanced(mode, 0, m_VertexBuffer->GetVertexCount(), instanceCount);
		}
		else
		{

			glDrawElementsInstanced(mode, indexCount, GL_UNSIGNED_INT, nullptr, instanceCount);

		}
	}
    void OpenGLGraphicsPipeline::SetVertexBuffer(Ref<VertexBuffer>& buffer)
    {
        m_VertexBuffer = std::dynamic_pointer_cast<OpenGLVertexBuffer>(buffer);
    }
    void OpenGLGraphicsPipeline::SetMaterial(Ref<Material>& material)
    {
        m_Material = std::dynamic_pointer_cast<OpenGLMaterial>(material);
    }
}