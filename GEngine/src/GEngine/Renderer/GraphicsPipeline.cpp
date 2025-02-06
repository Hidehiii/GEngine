#include "GEpch.h"
#include "GraphicsPipeline.h"
#include "GEngine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLGraphicsPipeline.h"
#include "Platform/Vulkan/VulkanGraphicsPipeline.h"

namespace GEngine
{
	Ref<GraphicsPipeline> GraphicsPipeline::Create(const Ref<Material>& material, const Ref<VertexBuffer>& vertexBuffer)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::OpenGL:		return CreateRef<OpenGLGraphicsPipeline>(material, vertexBuffer); break;
		case RendererAPI::API::Vulkan:		return CreateRef<VulkanGraphicsPipeline>(material, vertexBuffer); break;
		case RendererAPI::API::None:
		default:
			GE_CORE_ASSERT(false, "Unknow render api");
			break;
		}
		return nullptr;
	}
}