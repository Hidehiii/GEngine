#include "GEpch.h"
#include "StorageImage.h"
#include "GEngine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLStorageImage2D.h"
#include "Platform/Vulkan/VulkanStorageImage2D.h"
namespace GEngine
{
	Ref<StorageImage2D>	StorageImage2D::s_WhiteStorageImage2D = nullptr;

	Ref<StorageImage2D> StorageImage2D::Create(uint32_t width, uint32_t height, ComputeImage2DFormat format)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return CreateRef<OpenGLStorageImage2D>(width, height, format);
		case RendererAPI::API::Vulkan:  return CreateRef<VulkanStorageImage2D>(width, height, format);
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<StorageImage2D> StorageImage2D::White()
	{
		if (s_WhiteStorageImage2D == nullptr)
		{
			uint32_t data[4] = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
			s_WhiteStorageImage2D = StorageImage2D::Create(1, 1, ComputeImage2DFormat::RGBA32F);
			s_WhiteStorageImage2D->SetData(data, sizeof(uint32_t) * 4);
		}
		return s_WhiteStorageImage2D;
	}
}