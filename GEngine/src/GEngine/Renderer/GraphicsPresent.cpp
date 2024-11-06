#include "GEpch.h"
#include "GraphicsPresent.h"
#include "GEngine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLGraphicsPresent.h"
#include "Platform/Vulkan/VulkanGraphicsPresent.h"

namespace GEngine
{
	Scope<GraphicsPresent> GraphicsPresent::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return CreateScope<OpenGLGraphicsPresent>();
		case RendererAPI::API::Vulkan:  return CreateScope<VulkanGraphicsPresent>();
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}