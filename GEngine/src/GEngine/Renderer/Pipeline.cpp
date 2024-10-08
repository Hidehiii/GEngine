#include "GEpch.h"
#include "Pipeline.h"
#include "GEngine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLPipeline.h"
#include "Platform/Vulkan/VulkanPipeline.h"

namespace GEngine
{
	Ref<Pipeline> Pipeline::Create(const Ref<Material>& material, const Ref<VertexArray>& vertexArray, const Ref<VertexBuffer>& vertexBuffer)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::OpenGL:		return CreateRef<OpenGLPipeline>(material, vertexArray, vertexBuffer); break;
		case RendererAPI::API::Vulkan:		return CreateRef<VulkanPipeline>(material, vertexArray, vertexBuffer); break;
		case RendererAPI::API::None:
		default:
			GE_CORE_ASSERT(false, "Unknow render api");
			break;
		}
		return nullptr;
	}
}