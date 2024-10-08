#include "GEpch.h"
#include "FrameBuffer.h"
#include "GEngine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLFrameBuffer.h"

namespace GEngine
{
	Ref<FrameBuffer>	FrameBuffer::s_CurrentFrameBuffer;

    Ref<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecification& spec)
    {
        switch (Renderer::GetAPI())
        {
		case RendererAPI::API::None:    GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return CreateRef<OpenGLFrameBuffer>(spec);
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}