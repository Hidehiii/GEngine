#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/Material.h"

namespace GEngine
{
	class CommandBuffer;


	class GENGINE_API ComputePipeline
	{
	public:
		virtual ~ComputePipeline() = default;
		virtual Ref<Material>	GetMaterial() = 0;
		virtual void			SetMaterial(Ref<Material>& material) = 0;

		static Ref<ComputePipeline> Create(const Ref<Material>& material);

	protected:
		virtual void Compute(CommandBuffer* cmdBuffer, int pass, uint32_t x, uint32_t y, uint32_t z) = 0;

		friend class CommandBuffer;
	};
}


