#include "GEpch.h"
#include "OpenGLPipeline.h"
#include "GEngine/Renderer/RenderCommand.h"

namespace GEngine
{
    OpenGLPipeline::OpenGLPipeline(const Ref<Material>& material, const Ref<VertexBuffer>& vertexBuffer)
    {
        m_Material      = std::dynamic_pointer_cast<OpenGLMaterial>(material);
        m_VertexBuffer  = std::dynamic_pointer_cast<OpenGLVertexBuffer>(vertexBuffer);
    }
    OpenGLPipeline::~OpenGLPipeline()
    {
    }
    void OpenGLPipeline::Render(uint32_t instanceCount, uint32_t indexCount)
    {
        m_Material->Update();
        m_VertexBuffer->Bind();
        indexCount = indexCount > 0 ? indexCount : m_VertexBuffer->GetIndexBuffer()->GetCount(); 
        if (m_VertexBuffer->IsInstanceRendering())
        {
            switch (m_VertexBuffer->GetVertexTopologyType())
            {
            case VertexTopology::Triangle:
            {
                RenderCommand::DrawTriangleInstance(indexCount, instanceCount);
                break;
            }
            case VertexTopology::Line:
            {
                RenderCommand::DrawLines(indexCount);
                break;
            }
            case VertexTopology::Point:
            {
                RenderCommand::DrawPoints(indexCount);
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
                RenderCommand::DrawTriangles(indexCount);
                break;
            }
            case VertexTopology::Line:
            {
                RenderCommand::DrawLines(indexCount);
                break;
            }
            case VertexTopology::Point:
            {
                RenderCommand::DrawPoints(indexCount);
                break;
            }
            default:
                GE_CORE_ASSERT(false, "Unknow type");
                break;
            }
        }
    }
    void OpenGLPipeline::SetVertexBuffer(Ref<VertexBuffer>& buffer)
    {
        m_VertexBuffer = std::dynamic_pointer_cast<OpenGLVertexBuffer>(buffer);
    }
    void OpenGLPipeline::SetMaterial(Ref<Material>& material)
    {
        m_Material = std::dynamic_pointer_cast<OpenGLMaterial>(material);
    }
}