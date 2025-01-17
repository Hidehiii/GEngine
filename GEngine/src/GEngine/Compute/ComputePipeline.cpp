#include "GEpch.h"
#include "ComputePipeline.h"
#include "GEngine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLComputePipeline.h"
#include "Platform/Vulkan/VulkanComputePipeline.h"

namespace GEngine
{
	Ref<ComputePipeline> ComputePipeline::Create(const Ref<Material>& material)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::OpenGL:	return CreateRef<OpenGLComputePipeline>(material); break;
		case RendererAPI::API::Vulkan:	return CreateRef<VulkanComputePipeline>(material); break;
		case RendererAPI::API::None:
		default:
			GE_CORE_ASSERT(false, "Unknow render api");
			break;
		}
		return nullptr;
	}
}