#pragma once
#include "GEngine/Graphics/RenderPass.h"
#include <directx/d3dx12.h>

namespace GEngine
{
	struct RenderPassSpecificationForD3D12
	{
		std::vector<DXGI_FORMAT>								BackBufferFormat;
		bool													EnableDepthStencil;
		int														Samples;
		RenderPassOperation										Operation;
	};

	class GENGINE_API D3D12RenderPass : public RenderPass
	{
	public:
		D3D12RenderPass(const RenderPassSpecification& spec);
		D3D12RenderPass(const RenderPassSpecificationForD3D12& spec);

		virtual ~D3D12RenderPass() override;
	};
}


