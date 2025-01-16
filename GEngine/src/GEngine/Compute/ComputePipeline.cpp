#include "GEpch.h"
#include "ComputePipeline.h"
#include "GEngine/Renderer/Renderer.h"

namespace GEngine
{
	Ref<ComputePipeline> ComputePipeline::Create(const Ref<Material>& material)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::OpenGL:	return nullptr;
		case RendererAPI::API::Vulkan:	return nullptr;
		case RendererAPI::API::None:
		default:
			GE_CORE_ASSERT(false, "Unknow render api");
			break;
		}
		return nullptr;
	}
}