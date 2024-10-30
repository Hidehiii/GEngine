#include "GEpch.h"
#include "Graphics.h"
#include "GEngine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLGraphics.h"
#include "Platform/Vulkan/VulkanGraphics.h"

namespace GEngine
{
	Ref<Graphics> Graphics::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return CreateRef<OpenGLGraphics>();
		case RendererAPI::API::Vulkan:  return CreateRef<VulkanGraphics>();
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}