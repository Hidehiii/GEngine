#include "GEpch.h"
#include "D3D12RenderPass.h"

namespace GEngine
{
	D3D12RenderPass::D3D12RenderPass(const RenderPassSpecification& spec)
	{
		m_Specification = spec;
	}
	D3D12RenderPass::~D3D12RenderPass()
	{
	}
}