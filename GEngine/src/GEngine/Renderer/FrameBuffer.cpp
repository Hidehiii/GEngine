#include "GEpch.h"
#include "FrameBuffer.h"
#include "GEngine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLFrameBuffer.h"
#include "Platform/Vulkan/VulkanFrameBuffer.h"

namespace GEngine
{

    Ref<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecification& spec)
    {
        switch (Renderer::GetAPI())
        {
		case RendererAPI::API::None:    GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return CreateRef<OpenGLFrameBuffer>(spec);
		case RendererAPI::API::Vulkan:  return CreateRef<VulkanFrameBuffer>(spec);
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<FrameBuffer> FrameBuffer::Recreate(const Ref<FrameBuffer>& buffer, uint32_t width, uint32_t height)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return CreateRef<OpenGLFrameBuffer>(buffer, width, height);
		case RendererAPI::API::Vulkan:  return CreateRef<VulkanFrameBuffer>(buffer, width, height);
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<FrameBuffer> FrameBuffer::Recreate(const Ref<FrameBuffer>& buffer, Vector2 size)
	{
		return Recreate(buffer, size.x, size.y);
	}
}