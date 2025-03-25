#include "GEpch.h"
#include "StorageBuffer.h"
#include "GEngine/Graphics/Renderer.h"
#include "Platform/OpenGL/OpenGLStorageBuffer.h"
#include "Platform/Vulkan/VulkanStorageBuffer.h"

namespace GEngine
{
	Ref<StorageBuffer> StorageBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case GraphicsAPI::API::None:    GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case GraphicsAPI::API::OpenGL:  return CreateRef<OpenGLStorageBuffer>(size);
		case GraphicsAPI::API::Vulkan:  return CreateRef<VulkanStorageBuffer>(size);
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}