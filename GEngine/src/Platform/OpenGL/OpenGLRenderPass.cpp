#include "GEpch.h"
#include "Platform/OpenGL/OpenGLRenderPass.h"

namespace GEngine
{
    OpenGLRenderPass::OpenGLRenderPass(const RenderPassSpecification& spec)
    {
        m_Specification = spec;
    }
    OpenGLRenderPass::~OpenGLRenderPass()
    {
    }
}