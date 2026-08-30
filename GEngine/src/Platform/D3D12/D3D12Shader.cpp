#include "GEpch.h"
#include "Platform/D3D12/D3D12Shader.h"
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

			std::vector<D3D12_DESCRIPTOR_RANGE> resourceRanges;
			resourceRanges.reserve(cbuffers.size() + resources.size());
			for (const auto& cbuffer : cbuffers)
			{
				D3D12_DESCRIPTOR_RANGE range{};
				range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				range.NumDescriptors = 1;
				range.BaseShaderRegister = cbuffer.BindPoint;
				range.RegisterSpace = 0;
				range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				resourceRanges.push_back(range);
			}

			std::vector<D3D12_DESCRIPTOR_RANGE> samplerRanges;
			samplerRanges.reserve(resources.size());
			for (const auto& resource : resources)
			{
				if (resource.Type == SHADER_PROPERTY_TYPE_SAMPLER)
				{
					D3D12_DESCRIPTOR_RANGE range{};
					range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
					range.NumDescriptors = 1;
					range.BaseShaderRegister = resource.BindPoint;
					range.RegisterSpace = 0;
					range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
					samplerRanges.push_back(range);
					continue;
				}

				D3D12_DESCRIPTOR_RANGE range{};
				const bool isUav = resource.Type >= SHADER_PROPERTY_TYPE_STORAGE_IMAGE_UNKNOWN;
				range.RangeType = isUav ? D3D12_DESCRIPTOR_RANGE_TYPE_UAV : D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				range.NumDescriptors = 1;
				range.BaseShaderRegister = resource.BindPoint;
				range.RegisterSpace = 0;
				range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				resourceRanges.push_back(range);
			}

			std::vector<D3D12_ROOT_PARAMETER> rootParams;
			// Descriptor-table ranges are referenced by root parameters during serialization.
			// Build both range vectors first and keep parameter storage stable until serialization completes.
			rootParams.reserve(2);
			if (!resourceRanges.empty())
			{
				m_RootBindingLayouts[pass].ResourceTableRootIndex = static_cast<int>(rootParams.size());
				D3D12_ROOT_PARAMETER parameter{};
				parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
				parameter.DescriptorTable = { static_cast<UINT>(resourceRanges.size()), resourceRanges.data() };
				rootParams.push_back(parameter);
			}
			if (!samplerRanges.empty())
			{
				m_RootBindingLayouts[pass].SamplerTableRootIndex = static_cast<int>(rootParams.size());
				D3D12_ROOT_PARAMETER parameter{};
				parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
				parameter.DescriptorTable = { static_cast<UINT>(samplerRanges.size()), samplerRanges.data() };
				rootParams.push_back(parameter);
			}

			D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
			rootSignatureDesc.NumParameters = static_cast<UINT>(rootParams.size());
			rootSignatureDesc.pParameters = rootParams.data();
			rootSignatureDesc.NumStaticSamplers = 0;
			rootSignatureDesc.pStaticSamplers = nullptr;
			rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
			ID3DBlob* blob = nullptr;
			ID3DBlob* errorBlob = nullptr;
			D3D12_THROW_IF_FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &errorBlob));
			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignatures.at(pass))));
			if (errorBlob) errorBlob->Release();
			blob->Release();
		}
	}
}
