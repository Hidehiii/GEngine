#include "GEpch.h"
#include "GEngine/Compute/ComputePipeline.h"
#include "GEngine/Graphics/Graphics.h"

namespace GEngine
{
	Ref<ComputePipeline> ComputePipeline::Create(const Ref<Material>& material)
	{
		return Graphics::GetRenderDevice().CreateComputePipeline(material);
	}
}
