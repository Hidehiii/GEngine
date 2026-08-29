#include "GEpch.h"
#include "D3D12Shader.h"
#include "Platform/D3D12/D3D12Context.h"
#include "Platform/D3D12/D3D12Utils.h"

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
		m_CompiledByteCodes = shaders;
		m_ByteCodes.clear();
		m_ByteCodes.resize(m_CompiledByteCodes.size());
		for(int i = 0; i < m_CompiledByteCodes.size(); i++)
		{
			for (auto&& [stage, byte] : m_CompiledByteCodes[i])
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
		m_RootBindingLayouts.clear();
		m_RootBindingLayouts.resize(m_PassReflections.size());
		for (int pass = 0; pass < m_PassReflections.size(); pass++)
		{
			std::vector<ShaderReflectionCBufferInfo> cbuffers(
				m_PassReflections.at(pass).CBuffers.begin(), m_PassReflections.at(pass).CBuffers.end());
			std::sort(cbuffers.begin(), cbuffers.end(), [](const auto& left, const auto& right) { return left.BindPoint < right.BindPoint; });

			std::vector<ShaderReflectionResourceInfo> resources(
				m_PassReflections.at(pass).Resources.begin(), m_PassReflections.at(pass).Resources.end());
			std::sort(resources.begin(), resources.end(), [](const auto& left, const auto& right) { return left.BindPoint < right.BindPoint; });

			std::vector<CD3DX12_DESCRIPTOR_RANGE> resourceRanges;
			for (const auto& cbuffer : cbuffers)
			{
				CD3DX12_DESCRIPTOR_RANGE range;
				range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, cbuffer.BindPoint);
				resourceRanges.push_back(range);
			}

			std::vector<CD3DX12_DESCRIPTOR_RANGE> samplerRanges;
			for (const auto& resource : resources)
			{
				if (resource.Type == SHADER_PROPERTY_TYPE_SAMPLER)
				{
					CD3DX12_DESCRIPTOR_RANGE range;
					range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, resource.BindPoint);
					samplerRanges.push_back(range);
					continue;
				}

				CD3DX12_DESCRIPTOR_RANGE range;
				const bool isUav = resource.Type >= SHADER_PROPERTY_TYPE_STORAGE_IMAGE_UNKNOWN;
				range.Init(isUav ? D3D12_DESCRIPTOR_RANGE_TYPE_UAV : D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, resource.BindPoint);
				resourceRanges.push_back(range);
			}

			std::vector<CD3DX12_ROOT_PARAMETER> rootParams;
			if (!resourceRanges.empty())
			{
				m_RootBindingLayouts[pass].ResourceTableRootIndex = static_cast<int>(rootParams.size());
				rootParams.emplace_back();
				rootParams.back().InitAsDescriptorTable(static_cast<UINT>(resourceRanges.size()), resourceRanges.data());
			}
			if (!samplerRanges.empty())
			{
				m_RootBindingLayouts[pass].SamplerTableRootIndex = static_cast<int>(rootParams.size());
				rootParams.emplace_back();
				rootParams.back().InitAsDescriptorTable(static_cast<UINT>(samplerRanges.size()), samplerRanges.data());
			}

			CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
			rootSignatureDesc.Init(static_cast<UINT>(rootParams.size()), rootParams.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

			ID3DBlob* blob = nullptr;
			ID3DBlob* errorBlob = nullptr;
			D3D12_THROW_IF_FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &errorBlob));
			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignatures.at(pass))));
			if (errorBlob) errorBlob->Release();
			blob->Release();
		}
	}
}
