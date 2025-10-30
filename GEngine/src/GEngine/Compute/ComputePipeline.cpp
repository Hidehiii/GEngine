#include "GEpch.h"
#include "ComputePipeline.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/OpenGL/OpenGLComputePipeline.h"
#include "Platform/Vulkan/VulkanComputePipeline.h"

namespace GEngine
{
	Ref<ComputePipeline> ComputePipeline::Create(const Ref<Material>& material)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GraphicsAPI::GRAPHICS_API_OpenGL:	return CreateRef<OpenGLComputePipeline>(material); break;
		case GraphicsAPI::GRAPHICS_API_Vulkan:	return CreateRef<VulkanComputePipeline>(material); break;
		case GraphicsAPI::GRAPHICS_API_None:
		default:
			GE_CORE_ASSERT(false, "Unknow render api");
			break;
		}
		return nullptr;
	}
}