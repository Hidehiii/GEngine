#include "GEpch.h"
#include "GEngine/Graphics/GraphicsPipeline.h"
#include "GEngine/Graphics/Graphics.h"

namespace GEngine
{
	Ref<GraphicsPipeline> GraphicsPipeline::Create(const Ref<Material>& material, const Ref<VertexBuffer>& vertexBuffer)
	{
		return Graphics::GetRenderDevice().CreateGraphicsPipeline(material, vertexBuffer);
	}
}
