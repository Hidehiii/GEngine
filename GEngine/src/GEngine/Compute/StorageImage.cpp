#include "GEpch.h"
#include "StorageImage.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/OpenGL/OpenGLStorageImage2D.h"
#include "Platform/Vulkan/VulkanStorageImage2D.h"
namespace GEngine
{
	Ref<StorageImage2D> StorageImage2D::Create(uint32_t width, uint32_t height, ComputeImage2DFormat format)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GraphicsAPI::API::None:    GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case GraphicsAPI::API::OpenGL:  return CreateRef<OpenGLStorageImage2D>(width, height, format);
		case GraphicsAPI::API::Vulkan:  return CreateRef<VulkanStorageImage2D>(width, height, format);
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}