#include "GEpch.h"
#include "FrameBuffer.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/OpenGL/OpenGLFrameBuffer.h"
#include "Platform/Vulkan/VulkanFrameBuffer.h"

namespace GEngine
{

	Ref<FrameBuffer> FrameBuffer::Create(const Ref<RenderPass>& renderPass, const Vector2& size)
	{
		return Create(renderPass, size.x, size.y);
	}

	Ref<FrameBuffer> FrameBuffer::Create(const Ref<RenderPass>& renderPass, uint32_t width, uint32_t height)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_NONE:    GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!"); return nullptr;
		case GRAPHICS_API_OPENGL:  return CreateRef<OpenGLFrameBuffer>(renderPass, width, height);
		case GRAPHICS_API_VULKAN:  return CreateRef<VulkanFrameBuffer>(renderPass, width, height);
		}

		GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
		return nullptr;
	}

	Ref<FrameBuffer> FrameBuffer::Create(const Ref<FrameBuffer>& buffer, uint32_t width, uint32_t height)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_NONE:    GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!"); return nullptr;
		case GRAPHICS_API_OPENGL:  return CreateRef<OpenGLFrameBuffer>(buffer, width, height);
		case GRAPHICS_API_VULKAN:  return CreateRef<VulkanFrameBuffer>(buffer, width, height);
		}

		GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
		return nullptr;
	}

	Ref<FrameBuffer> FrameBuffer::Create(const Ref<FrameBuffer>& buffer, Vector2 size)
	{
		return Create(buffer, size.x, size.y);
	}
}