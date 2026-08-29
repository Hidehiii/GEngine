#include "GEpch.h"
#include "StorageBuffer.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/OpenGL/OpenGLStorageBuffer.h"
#include "Platform/Vulkan/VulkanStorageBuffer.h"
#include "Platform/D3D12/D3D12StorageBuffer.h"

namespace GEngine
{
	Ref<StorageBuffer> StorageBuffer::Create(uint32_t size)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_NONE:    GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case GRAPHICS_API_OPENGL:  return CreateRef<OpenGLStorageBuffer>(size);
		case GRAPHICS_API_VULKAN:  return CreateRef<VulkanStorageBuffer>(size);
		case GRAPHICS_API_DIRECT3DX12: return CreateRef<D3D12StorageBuffer>(size);
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
