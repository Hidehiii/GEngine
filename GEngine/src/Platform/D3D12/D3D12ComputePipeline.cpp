#include "GEpch.h"
#include "Platform/D3D12/D3D12ComputePipeline.h"
#include "Platform/D3D12/D3D12CommandBuffer.h"
#include "Platform/D3D12/D3D12Context.h"
#include "Platform/D3D12/D3D12Shader.h"
#include "Platform/D3D12/D3D12Utils.h"
#include "GEngine/Graphics/Graphics.h"

namespace GEngine
{
	D3D12ComputePipeline::D3D12ComputePipeline(const Ref<Material>& material)
		: m_Material(std::dynamic_pointer_cast<D3D12Material>(material))
	{
		GE_CORE_ASSERT(m_Material, "D3D12 compute pipelines require a D3D12 material.");
	}

	Ref<Material> D3D12ComputePipeline::GetMaterial()
	{
		return m_Material;
	}

	void D3D12ComputePipeline::SetMaterial(Ref<Material>& material)
	{
		m_Material = std::dynamic_pointer_cast<D3D12Material>(material);
		GE_CORE_ASSERT(m_Material, "D3D12 compute pipelines require a D3D12 material.");
		m_PipelineStates.clear();
	}

	Microsoft::WRL::ComPtr<ID3D12PipelineState> D3D12ComputePipeline::GetPipelineState(uint32_t pass)
	{
		auto cached = m_PipelineStates.find(pass);
		if (cached != m_PipelineStates.end())
			return cached->second;

		auto shader = std::dynamic_pointer_cast<D3D12Shader>(m_Material->GetShader());
		GE_CORE_ASSERT(shader && pass < shader->GetByteCodes().size(), "Compute shader pass is invalid.");
		auto byteCode = shader->GetByteCodes().at(pass).find(SHADER_STAGE_COMPUTE);
		GE_CORE_ASSERT(byteCode != shader->GetByteCodes().at(pass).end(), "The shader pass has no compute stage.");

		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = shader->GetRootSignatures().at(pass).Get();
		desc.CS = byteCode->second;

		Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
		D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pipelineState)));
		m_PipelineStates.emplace(pass, pipelineState);
		return pipelineState;
	}

	void D3D12ComputePipeline::PrepareCompute(CommandBuffer* cmdBuffer, uint32_t pass)
	{
		m_Material->Update(cmdBuffer, pass);
		auto commandList = static_cast<D3D12CommandBuffer*>(cmdBuffer)->GetCommandList();
		auto shader = std::dynamic_pointer_cast<D3D12Shader>(m_Material->GetShader());

		commandList->SetComputeRootSignature(shader->GetRootSignatures().at(pass).Get());
		commandList->SetPipelineState(GetPipelineState(pass).Get());

		ID3D12DescriptorHeap* descriptorHeaps[] =
		{
			D3D12Context::Get()->GetCbvSrvUavDescriptorHeap(Graphics::GetFrame()),
			D3D12Context::Get()->GetSamplerDescriptorHeap(Graphics::GetFrame())
		};
		commandList->SetDescriptorHeaps(2, descriptorHeaps);

		const auto& layout = shader->GetRootBindingLayout(pass);
		const auto& resourceHeap = m_Material->GetCbvSrvUavHeaps().at(pass);
		if (layout.ResourceTableRootIndex >= 0 && resourceHeap.IsValid())
			commandList->SetComputeRootDescriptorTable(layout.ResourceTableRootIndex, resourceHeap.GpuHandles.at(Graphics::GetFrame()));

		const auto& samplerHeap = m_Material->GetSamplerHeaps().at(pass);
		if (layout.SamplerTableRootIndex >= 0 && samplerHeap.IsValid())
			commandList->SetComputeRootDescriptorTable(layout.SamplerTableRootIndex, samplerHeap.GpuHandles.at(Graphics::GetFrame()));
	}

	void D3D12ComputePipeline::Compute(CommandBuffer* cmdBuffer, uint32_t pass, uint32_t x, uint32_t y, uint32_t z)
	{
		PrepareCompute(cmdBuffer, pass);
		static_cast<D3D12CommandBuffer*>(cmdBuffer)->GetCommandList()->Dispatch(x, y, z);
	}
}
