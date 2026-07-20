#include "GEpch.h"
#include "GraphicsPipeline.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/OpenGL/OpenGLGraphicsPipeline.h"
#include "Platform/Vulkan/VulkanGraphicsPipeline.h"
#include "Platform/D3D12/D3D12GraphicsPipeline.h"

namespace GEngine
{
	Ref<GraphicsPipeline> GraphicsPipeline::Create(const Ref<Material>& material, const Ref<VertexBuffer>& vertexBuffer)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_OPENGL:		return CreateRef<OpenGLGraphicsPipeline>(material, vertexBuffer); break;
		case GRAPHICS_API_VULKAN:		return CreateRef<VulkanGraphicsPipeline>(material, vertexBuffer); break;
		case GRAPHICS_API_DIRECT3DX12:	return CreateRef<D3D12GraphicsPipeline>(material, vertexBuffer); break;
		case GRAPHICS_API_NONE:
		default:
			GE_CORE_ASSERT(false, "Unknow render api");
			break;
		}
		return nullptr;
	}
}
