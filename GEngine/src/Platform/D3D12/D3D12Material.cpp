#include "GEpch.h"
#include "D3D12Material.h"
#include "Platform/D3D12/D3D12Context.h"
#include "Platform/D3D12/D3D12Texture2D.h"

namespace GEngine
{
	D3D12Material::D3D12Material(const Ref<Shader>& shader, const std::string& name)
	{
		m_Shader = std::dynamic_pointer_cast<D3D12Shader>(shader);
		m_ShaderBase = shader;
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

	Buffer D3D12Material::SetUniformBuffer(const uint32_t& pass, const uint32_t& bindPoint, const Buffer& buffer, const Ref<UniformBuffer>& buf)
	{
		GE_CORE_ASSERT(pass < m_Passes.size(), "Pass index out of range!");
		GE_CORE_ASSERT(m_Passes.at(pass).CBuffers.find(bindPoint) != m_Passes.at(pass).CBuffers.end(), "CBuffer bind point not found!");
		Buffer old = m_Passes.at(pass).CBuffers.at(bindPoint);
		m_Passes.at(pass).CBuffers.at(bindPoint) = buffer;
		return old;
	}

	void D3D12Material::Update(CommandBuffer* cmdBuffer, const uint32_t& pass)
	{
		GE_CORE_ASSERT(pass < m_Passes.size(), "Pass index out of range!");
		GE_CORE_ASSERT(m_ConstantBuffers.size() > pass, "Pass index out of range for constant buffers!");
		for (auto& [bindPoint, buffer] : m_ConstantBuffers.at(pass))
		{
			auto& cpuBuffer = m_Passes.at(pass).CBuffers.at(bindPoint);
			if (cpuBuffer.Size > 0)
			{
				buffer->SetData(cpuBuffer.Data, cpuBuffer.Size);
			}
		}
	}

	void D3D12Material::CreateDescriptorHeap()
	{
		m_CbvSrvUavHeaps.clear();
		m_CbvSrvUavHeaps.resize(m_Shader->GetPassReflections().size());
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
			m_CbvSrvUavHeaps[i] = D3D12Context::Get()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, CbvSrvUavCount);
		}
		// write descriptor heap for each pass
		for (int pass = 0; pass < m_CbvSrvUavHeaps.size(); pass++)
		{
			UINT descStep = D3D12Context::Get()->GetCbvSrvUavDescriptorIncrementSize();
			// each frame 
			for (int frame = 0; frame < m_CbvSrvUavHeaps.at(pass).CpuHandles.size(); frame++)
			{
				CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_CbvSrvUavHeaps.at(pass).CpuHandles.at(frame));
				uint32_t slotIndex = 0;
				// material cbv
				for (auto& [bindpoint, buffer] : m_ConstantBuffers.at(pass))
				{
					// offset the cpu handle by the slot index
					cpuHandle.Offset(slotIndex, descStep);
					slotIndex++;

					D3D12Context::Get()->GetDevice()->CreateConstantBufferView(&buffer->GetConstantBufferViewDesc(), cpuHandle);
				}
				// other resources
				for (auto& [name, prop] : m_Passes.at(pass).ResourceProperties)
				{
					// offset the cpu handle by the slot index
					cpuHandle.Offset(slotIndex, descStep);
					slotIndex++;

					auto type = m_Shader->GetPropertyType(name);
					switch (type)
					{
					case SHADER_PROPERTY_TYPE_SAMPLER_TEXTURE_1D: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_SAMPLER_TEXTURE_2D: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_SAMPLER_TEXTURE_3D: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_SAMPLER_TEXTURE_1D_ARRAY: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_SAMPLER_TEXTURE_2D_ARRAY: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_SAMPLER_TEXTURE_3D_ARRAY: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_SAMPLER_TEXTURE_CUBE: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_SAMPLER_TEXTURE_CUBE_ARRAY: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_SAMPLER: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_TEXTURE_1D: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_TEXTURE_2D:
					{
						D3D12Context::Get()->GetDevice()->CreateShaderResourceView(
							(*((Ref<D3D12Texture2D>*)prop.Ptr))->GetResource().Get(),
							&((*((Ref<D3D12Texture2D>*)prop.Ptr))->GetShaderResourceViewDesc()),
							cpuHandle);
						break;
					}
					case SHADER_PROPERTY_TYPE_TEXTURE_3D: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_TEXTURE_1D_ARRAY: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_TEXTURE_2D_ARRAY: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_TEXTURE_3D_ARRAY: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_TEXTURE_CUBE: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_TEXTURE_CUBE_ARRAY: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_RWTEXTURE_1D: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_RWTEXTURE_2D: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_RWTEXTURE_3D: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_RWTEXTURE_CUBE: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_RWTEXTURE_1D_ARRAY: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_RWTEXTURE_2D_ARRAY: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_RWTEXTURE_CUBE_ARRAY: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_RWBUFFER: GE_CORE_ASSERT(false, "Unsupport now!");
					case SHADER_PROPERTY_TYPE_RWBUFFER_DYNAMIC: GE_CORE_ASSERT(false, "Unsupport now!");
					default:
						GE_CORE_ASSERT(false, "Unsupport now!");
						break;
					}
				}
			}
		}
	}
}
