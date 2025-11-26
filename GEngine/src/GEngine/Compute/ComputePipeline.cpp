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
		case GRAPHICS_API_OPENGL:	return CreateRef<OpenGLComputePipeline>(material); break;
		case GRAPHICS_API_VULKAN:	return CreateRef<VulkanComputePipeline>(material); break;
		case GRAPHICS_API_DIRECT3DX12:
		case GRAPHICS_API_NONE:
		default:
			GE_CORE_ASSERT(false, "Unknow render api");
			break;
		}
		return nullptr;
	}
}