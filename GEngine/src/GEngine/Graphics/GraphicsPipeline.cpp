#include "GEpch.h"
#include "GraphicsPipeline.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/OpenGL/OpenGLGraphicsPipeline.h"
#include "Platform/Vulkan/VulkanGraphicsPipeline.h"

namespace GEngine
{
	Ref<GraphicsPipeline> GraphicsPipeline::Create(const Ref<Material>& material, const Ref<VertexBuffer>& vertexBuffer)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GraphicsAPI::API::OpenGL:		return CreateRef<OpenGLGraphicsPipeline>(material, vertexBuffer); break;
		case GraphicsAPI::API::Vulkan:		return CreateRef<VulkanGraphicsPipeline>(material, vertexBuffer); break;
		case GraphicsAPI::API::None:
		default:
			GE_CORE_ASSERT(false, "Unknow render api");
			break;
		}
		return nullptr;
	}
}