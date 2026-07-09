#include "GEpch.h"
#include "D3D12Material.h"
#include "Platform/D3D12/D3D12Context.h"

namespace GEngine
{
	D3D12Material::D3D12Material(const Ref<Shader>& shader, const std::string& name)
	{
		m_Shader = std::dynamic_pointer_cast<D3D12Shader>(shader);
		GE_CORE_ASSERT(m_Shader, "Shader is null!");
		m_Name = name.empty() ? m_Shader->GetShaderName() + " Material" : name;
		std::vector<std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>>> sizes = InitializePassPropertiesMemory(shader);
		// create constant buffers for each pass
		m_ConstantBuffers.clear();
		for (auto& pass : sizes)
		{
			std::unordered_map<uint32_t, Ref<D3D12UniformBuffer>> cbuffers;
			for (auto& [bindPoint, info] : pass) // info { size, count }
			{
				if (info.first > 0)
				{
					Ref<D3D12UniformBuffer> cb = CreateRef<D3D12UniformBuffer>(info.first, info.second);
					cbuffers[bindPoint] = cb;
				}
				
			}
			m_ConstantBuffers.push_back(cbuffers);
		}

		CreateDescriptorHeap();
	}

	void D3D12Material::CreateDescriptorHeap()
	{
		m_DescriptorHeaps.resize(m_Passes.size());
		// create descriptor heap for each pass
		for (int i = 0; i < m_Shader->GetPassReflections().size(); i++)
		{
			// calculate the number of CBV, SRV, UAV for this pass
			uint32_t CbvSrvUavCount = 0;
			for (auto& resource : m_Shader->GetPassReflections().at(i).Resources)
			{
				if (resource.Type != SHADER_PROPERTY_TYPE_SAMPLER)
					CbvSrvUavCount++;
			}

			D3D12_DESCRIPTOR_HEAP_DESC	heapDesc = {};
			heapDesc.Type				= D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			heapDesc.NumDescriptors		= CbvSrvUavCount;
			heapDesc.Flags				= D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

			D3D12Context::Get()->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DescriptorHeaps.at(i)));
		}
		// write descriptor heap for each pass
		for (int i = 0; i < m_DescriptorHeaps.size(); i++)
		{
			UINT descStep = D3D12Context::Get()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_DescriptorHeaps.at(i)->GetCPUDescriptorHandleForHeapStart());

			// material cbv
		}
	}
}