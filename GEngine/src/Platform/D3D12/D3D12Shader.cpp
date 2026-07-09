#include "GEpch.h"
#include "D3D12Shader.h"
#include "Platform/D3D12/D3D12Context.h"

namespace GEngine
{
	D3D12Shader::D3D12Shader(const std::string& path)
	{
		InitializeShader(path, GE_BIND_CLASS_FUNCTION_LAMBDA(ProcessMachineCode));
		CreateRootSignatures();
	}

	D3D12Shader::~D3D12Shader()
	{
	}

	void D3D12Shader::ProcessMachineCode(const std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& shaders)
	{
		m_ByteCodes.clear();
		m_ByteCodes.resize(shaders.size());
		for(int i = 0; i < shaders.size(); i++)
		{
			for (auto&& [stage, byte] : shaders[i])
			{
				D3D12_SHADER_BYTECODE		byteCode{};
				byteCode.BytecodeLength		= byte.size();
				byteCode.pShaderBytecode	= byte.data();

				m_ByteCodes.at(i)[stage]	= byteCode;
			}
		}
	}

	void D3D12Shader::CreateRootSignatures()
	{
		m_RootSignatures.clear();
		m_RootSignatures.resize(m_PassReflections.size());
		for (int pass = 0; pass < m_PassReflections.size(); pass++)
		{
			std::vector<CD3DX12_DESCRIPTOR_RANGE> ranges;
			for (auto& cbuffer : m_PassReflections.at(pass).CBuffers)
			{
				CD3DX12_DESCRIPTOR_RANGE range;
				range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, cbuffer.BindPoint);
				ranges.push_back(range);
			}
			for (auto& resource : m_PassReflections.at(pass).Resources)
			{
				// skip sampler, because we don't need to create a descriptor range for sampler, we will create a static sampler instead
				if (resource.Type == SHADER_PROPERTY_TYPE_SAMPLER)
					continue;
				CD3DX12_DESCRIPTOR_RANGE range;
				range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, resource.BindPoint);
				ranges.push_back(range);
			}
			// we assume that all the resource are in space 0, 
			// TODO: if we have resources in different spaces, we need to create a descriptor range for each space
			CD3DX12_ROOT_PARAMETER rootParam;
			rootParam.InitAsDescriptorTable(ranges.size(), ranges.data());

			CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
			rootSignatureDesc.Init(1, &rootParam);

			ID3DBlob* blob = nullptr;
			D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr);

			D3D12Context::Get()->GetDevice()->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignatures.at(pass)));
			blob->Release();
		}
	}
}