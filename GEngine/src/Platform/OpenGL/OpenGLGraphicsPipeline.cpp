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
	void OpenGLGraphicsPipeline::Render(CommandBuffer* cmdBuffer, const Ref<FrameBuffer>& frameBuffer, std::string pass, uint32_t instanceCount, uint32_t indexCount)
	{
		m_Material->Update(cmdBuffer, pass);
		m_VertexBuffer->Bind(cmdBuffer);
		indexCount = indexCount > 0 ? indexCount : m_VertexBuffer->GetIndexBuffer()->GetCount();
		if (m_VertexBuffer->IsInstanceRendering())
		{
			switch (m_VertexBuffer->GetVertexTopologyType())
			{
			case VertexTopology::Triangle:
			{
				glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
				break;
			}
			case VertexTopology::Line:
			{
				glDrawElementsInstanced(GL_LINES, indexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
				break;
			}
			case VertexTopology::Point:
			{
				glDrawElementsInstanced(GL_POINTS, indexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
				break;
			}
			default:
				GE_CORE_ASSERT(false, "Unknow type");
				break;
			}
		}
		else
		{
			switch (m_VertexBuffer->GetVertexTopologyType())
			{
			case VertexTopology::Triangle:
			{
				glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
				break;
			}
			case VertexTopology::Line:
			{
				glDrawElements(GL_LINES, indexCount, GL_UNSIGNED_INT, nullptr);
				break;
			}
			case VertexTopology::Point:
			{
				glDrawElements(GL_POINTS, indexCount, GL_UNSIGNED_INT, nullptr);
				break;
			}
			default:
				GE_CORE_ASSERT(false, "Unknow type");
				break;
			}
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