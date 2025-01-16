#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/Material.h"
namespace GEngine
{
	class GENGINE_API ComputePipeline
	{
	public:
		virtual ~ComputePipeline() = default;
		virtual void Compute(uint32_t x, uint32_t y, uint32_t z) = 0;
		virtual Ref<Material> GetMaterial() = 0;
		virtual void SetMaterial(Ref<Material>& material) = 0;
		static Ref<ComputePipeline> Create(const Ref<Material>& material);
	};
}


