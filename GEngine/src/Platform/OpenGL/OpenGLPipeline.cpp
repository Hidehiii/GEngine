#include "GEpch.h"
#include "OpenGLPipeline.h"

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
    void OpenGLPipeline::Bind()
    {
        m_Material->UploadData();
    }
    void OpenGLPipeline::Unbind()
    {
        
    }
}