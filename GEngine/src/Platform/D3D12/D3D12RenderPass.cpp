#include "GEpch.h"
#include "D3D12RenderPass.h"

namespace GEngine
{
	D3D12RenderPass::D3D12RenderPass(const RenderPassSpecification& spec)
	{
		m_Specification = spec;
	}
	D3D12RenderPass::D3D12RenderPass(const RenderPassSpecificationForD3D12& spec)
	{
		m_Specification.Samples		= spec.Samples;
		m_Specification.Operation	= spec.Operation;
	}
	D3D12RenderPass::~D3D12RenderPass()
	{
	}
}