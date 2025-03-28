#pragma once
#include "GEngine/Graphics/RenderPass.h"

namespace GEngine
{
	class GENGINE_API D3D12RenderPass : public RenderPass
	{
	public:
		D3D12RenderPass(const RenderPassSpecification& spec);

		virtual ~D3D12RenderPass() override;
	};
}


