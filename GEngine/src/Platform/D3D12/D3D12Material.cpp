#include "GEpch.h"
#include "Platform/D3D12/D3D12Material.h"
#include "Platform/D3D12/D3D12Context.h"
#include "Platform/D3D12/D3D12Texture2D.h"
#include "Platform/D3D12/D3D12Sampler.h"
#include "Platform/D3D12/D3D12StorageBuffer.h"
#include "Platform/D3D12/D3D12StorageImage2D.h"
#include "Platform/D3D12/D3D12Texture2DArray.h"
#include "Platform/D3D12/D3D12CubeMap.h"
#include "Platform/D3D12/D3D12TextureCombineSampler.h"

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

	D3D12Material::~D3D12Material()
	{
		if (!D3D12Context::Get())
			return;

		for (const auto& allocation : m_CbvSrvUavHeaps)
			if (allocation.IsValid()) D3D12Context::Get()->FreeDescriptor(allocation);
		for (const auto& allocation : m_SamplerHeaps)
			if (allocation.IsValid()) D3D12Context::Get()->FreeDescriptor(allocation);
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
		for (const auto& allocation : m_CbvSrvUavHeaps)
			if (allocation.IsValid()) D3D12Context::Get()->FreeDescriptor(allocation);
		for (const auto& allocation : m_SamplerHeaps)
			if (allocation.IsValid()) D3D12Context::Get()->FreeDescriptor(allocation);

		m_CbvSrvUavHeaps.clear();
		m_CbvSrvUavHeaps.resize(m_Shader->GetPassReflections().size());
		m_SamplerHeaps.clear();
		m_SamplerHeaps.resize(m_Shader->GetPassReflections().size());

		// create descriptor heap for each pass
		for (int i = 0; i < m_Shader->GetPassReflections().size(); i++)
		{
			uint32_t cbvSrvUavCount = static_cast<uint32_t>(m_Shader->GetPassReflections().at(i).CBuffers.size());
			uint32_t samplerCount = 0;
			for (const auto& resource : m_Shader->GetPassReflections().at(i).Resources)
			{
				resource.Type == SHADER_PROPERTY_TYPE_SAMPLER ? ++samplerCount : ++cbvSrvUavCount;
			}
			if (cbvSrvUavCount > 0)
				m_CbvSrvUavHeaps[i] = D3D12Context::Get()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, cbvSrvUavCount);
			if (samplerCount > 0)
				m_SamplerHeaps[i] = D3D12Context::Get()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, samplerCount);
		}

		for (uint32_t pass = 0; pass < m_Passes.size(); pass++)
		{
			std::vector<ShaderReflectionCBufferInfo> reflectedCBuffers(
				m_Shader->GetPassReflections().at(pass).CBuffers.begin(), m_Shader->GetPassReflections().at(pass).CBuffers.end());
			std::sort(reflectedCBuffers.begin(), reflectedCBuffers.end(), [](const auto& left, const auto& right) { return left.BindPoint < right.BindPoint; });

			std::vector<ShaderReflectionResourceInfo> resources(
				m_Shader->GetPassReflections().at(pass).Resources.begin(), m_Shader->GetPassReflections().at(pass).Resources.end());
			std::sort(resources.begin(), resources.end(), [](const auto& left, const auto& right) { return left.BindPoint < right.BindPoint; });

			const auto& resourceHeap = m_CbvSrvUavHeaps.at(pass);
			for (uint32_t frame = 0; frame < resourceHeap.CpuHandles.size(); frame++)
			{
				uint32_t slotIndex = 0;
				for (const auto& cbuffer : reflectedCBuffers)
				{
					CD3DX12_CPU_DESCRIPTOR_HANDLE handle(resourceHeap.CpuHandles.at(frame), slotIndex++, D3D12Context::Get()->GetCbvSrvUavDescriptorIncrementSize());
					auto buffer = m_ConstantBuffers.at(pass).find(cbuffer.BindPoint);
					if (buffer != m_ConstantBuffers.at(pass).end())
						D3D12Context::Get()->GetDevice()->CreateConstantBufferView(&buffer->second->GetConstantBufferViewDesc(), handle);
					else
						D3D12Context::Get()->GetDevice()->CreateConstantBufferView(nullptr, handle);
				}
				for (const auto& resource : resources)
				{
					if (resource.Type == SHADER_PROPERTY_TYPE_SAMPLER)
						continue;
					CD3DX12_CPU_DESCRIPTOR_HANDLE handle(resourceHeap.CpuHandles.at(frame), slotIndex++, D3D12Context::Get()->GetCbvSrvUavDescriptorIncrementSize());
					auto property = m_Passes.at(pass).ResourceProperties.find(resource.Name);
					if (resource.Type == SHADER_PROPERTY_TYPE_TEXTURE_2D && property != m_Passes.at(pass).ResourceProperties.end() && property->second.Ptr)
					{
						auto texture = std::dynamic_pointer_cast<D3D12Texture2D>(*static_cast<Ref<Texture2D>*>(property->second.Ptr));
						GE_CORE_ASSERT(texture, "D3D12 materials require D3D12 textures.");
						D3D12Context::Get()->GetDevice()->CreateShaderResourceView(
							texture->GetResource().Get(), &texture->GetShaderResourceViewDesc(), handle);
					}
					else if (resource.Type == SHADER_PROPERTY_TYPE_TEXTURE_2D_ARRAY && property != m_Passes.at(pass).ResourceProperties.end() && property->second.Ptr)
					{
						auto texture = std::dynamic_pointer_cast<D3D12Texture2DArray>(*static_cast<Ref<Texture2DArray>*>(property->second.Ptr));
						GE_CORE_ASSERT(texture, "D3D12 materials require D3D12 texture arrays.");
						D3D12Context::Get()->GetDevice()->CreateShaderResourceView(texture->GetResource(), &texture->GetShaderResourceViewDesc(), handle);
					}
					else if (resource.Type == SHADER_PROPERTY_TYPE_TEXTURE_CUBE && property != m_Passes.at(pass).ResourceProperties.end() && property->second.Ptr)
					{
						auto texture = std::dynamic_pointer_cast<D3D12CubeMap>(*static_cast<Ref<CubeMap>*>(property->second.Ptr));
						GE_CORE_ASSERT(texture, "D3D12 materials require D3D12 cube maps.");
						D3D12Context::Get()->GetDevice()->CreateShaderResourceView(texture->GetResource(), &texture->GetShaderResourceViewDesc(), handle);
					}
					else if (resource.Type == SHADER_PROPERTY_TYPE_SAMPLER_TEXTURE_2D && property != m_Passes.at(pass).ResourceProperties.end() && property->second.Ptr)
					{
						auto combined = std::dynamic_pointer_cast<D3D12Texture2DCombineSampler>(*static_cast<Ref<Texture2DCombineSampler>*>(property->second.Ptr));
						auto texture = combined ? std::dynamic_pointer_cast<D3D12Texture2D>(combined->GetTexture()) : nullptr;
						GE_CORE_ASSERT(texture, "D3D12 combined textures require a D3D12 texture.");
						D3D12Context::Get()->GetDevice()->CreateShaderResourceView(texture->GetResource().Get(), &texture->GetShaderResourceViewDesc(), handle);
					}
					else if (resource.Type == SHADER_PROPERTY_TYPE_SAMPLER_TEXTURE_CUBE && property != m_Passes.at(pass).ResourceProperties.end() && property->second.Ptr)
					{
						auto combined = std::dynamic_pointer_cast<D3D12CubeMapCombineSampler>(*static_cast<Ref<CubeMapCombineSampler>*>(property->second.Ptr));
						auto texture = combined ? std::dynamic_pointer_cast<D3D12CubeMap>(combined->GetCubeMap()) : nullptr;
						GE_CORE_ASSERT(texture, "D3D12 combined cube maps require a D3D12 cube map.");
						D3D12Context::Get()->GetDevice()->CreateShaderResourceView(texture->GetResource(), &texture->GetShaderResourceViewDesc(), handle);
					}
					else if (resource.Type == SHADER_PROPERTY_TYPE_STORAGE_IMAGE_2D && property != m_Passes.at(pass).ResourceProperties.end() && property->second.Ptr)
					{
						auto image = std::dynamic_pointer_cast<D3D12StorageImage2D>(*static_cast<Ref<StorageImage2D>*>(property->second.Ptr));
						GE_CORE_ASSERT(image, "D3D12 materials require D3D12 storage images.");
						D3D12Context::Get()->GetDevice()->CreateUnorderedAccessView(image->GetResource(), nullptr, &image->GetUnorderedAccessViewDesc(), handle);
					}
					else if (resource.Type == SHADER_PROPERTY_TYPE_STORAGE_BUFFER && property != m_Passes.at(pass).ResourceProperties.end() && property->second.Ptr)
					{
						auto buffer = std::dynamic_pointer_cast<D3D12StorageBuffer>(*static_cast<Ref<StorageBuffer>*>(property->second.Ptr));
						GE_CORE_ASSERT(buffer, "D3D12 materials require D3D12 storage buffers.");
						D3D12Context::Get()->GetDevice()->CreateUnorderedAccessView(buffer->GetResource(), nullptr, &buffer->GetUnorderedAccessViewDesc(), handle);
					}
					else if (resource.Type >= SHADER_PROPERTY_TYPE_STORAGE_IMAGE_UNKNOWN)
					{
						D3D12Context::Get()->GetDevice()->CreateUnorderedAccessView(nullptr, nullptr, nullptr, handle);
					}
					else
					{
						D3D12Context::Get()->GetDevice()->CreateShaderResourceView(nullptr, nullptr, handle);
					}
				}
			}

			const auto& samplerHeap = m_SamplerHeaps.at(pass);
			for (uint32_t frame = 0; frame < samplerHeap.CpuHandles.size(); frame++)
			{
				uint32_t slotIndex = 0;
				for (const auto& resource : resources)
				{
					if (resource.Type != SHADER_PROPERTY_TYPE_SAMPLER)
						continue;
					CD3DX12_CPU_DESCRIPTOR_HANDLE handle(samplerHeap.CpuHandles.at(frame), slotIndex++, D3D12Context::Get()->GetSamplerDescriptorIncrementSize());
					auto property = m_Passes.at(pass).ResourceProperties.find(resource.Name);
					if (property != m_Passes.at(pass).ResourceProperties.end() && property->second.Ptr)
					{
						auto sampler = std::dynamic_pointer_cast<D3D12Sampler>(*static_cast<Ref<Sampler>*>(property->second.Ptr));
						GE_CORE_ASSERT(sampler, "D3D12 materials require D3D12 samplers.");
						D3D12Context::Get()->GetDevice()->CreateSampler(&sampler->GetDescriptor(), handle);
					}
					else
					{
						D3D12_SAMPLER_DESC defaultSampler{};
						defaultSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
						defaultSampler.AddressU = defaultSampler.AddressV = defaultSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
						defaultSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
						defaultSampler.MaxLOD = D3D12_FLOAT32_MAX;
						D3D12Context::Get()->GetDevice()->CreateSampler(&defaultSampler, handle);
					}
				}
			}
		}
	}
}
