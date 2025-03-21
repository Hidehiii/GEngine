#include "GEpch.h"
#include "OpenGLGraphicsPipeline.h"
#include "GEngine/Renderer/RenderCommand.h"

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
    void OpenGLGraphicsPipeline::Render(CommandBuffer* cmdBuffer, const Ref<FrameBuffer>& frameBuffer, uint32_t instanceCount, uint32_t indexCount)
    {
		m_Material->Update(cmdBuffer);
		m_VertexBuffer->Bind(cmdBuffer);
		indexCount = indexCount > 0 ? indexCount : m_VertexBuffer->GetIndexBuffer()->GetCount();
		if (m_VertexBuffer->IsInstanceRendering())
		{
			switch (m_VertexBuffer->GetVertexTopologyType())
			{
			case VertexTopology::Triangle:
			{
				RenderCommand::DrawTriangleInstance(cmdBuffer, indexCount, instanceCount);
				break;
			}
			case VertexTopology::Line:
			{
				RenderCommand::DrawLines(cmdBuffer, indexCount);
				break;
			}
			case VertexTopology::Point:
			{
				RenderCommand::DrawPoints(cmdBuffer, indexCount);
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
				RenderCommand::DrawTriangles(cmdBuffer, indexCount);
				break;
			}
			case VertexTopology::Line:
			{
				RenderCommand::DrawLines(cmdBuffer, indexCount);
				break;
			}
			case VertexTopology::Point:
			{
				RenderCommand::DrawPoints(cmdBuffer, indexCount);
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