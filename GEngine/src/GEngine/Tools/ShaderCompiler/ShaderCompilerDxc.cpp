#include "GEpch.h"
#include "ShaderCompilerDxc.h"
#include <d3d12shader.h>

namespace GEngine
{
	ShaderCompilerDxc::ShaderCompilerDxc()
	{
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_Utils));
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_Compiler));
		m_Utils->CreateDefaultIncludeHandler(&m_IncludeHandler);
	}

	bool ShaderCompilerDxc::Compile(const std::string& source, std::vector<LPWSTR>& args, std::vector<uint8_t>& output)
	{
		DxcBuffer				sourceBuffer;
		sourceBuffer.Ptr = source.data();
		sourceBuffer.Size = source.size();
		sourceBuffer.Encoding = DXC_CP_ACP;

		IDxcResult* result;
		HRESULT hr = m_Compiler->Compile(&sourceBuffer, args.data(), (uint32_t)args.size(), m_IncludeHandler, IID_PPV_ARGS(&result));
		if (FAILED(hr))
		{
			GE_CORE_ERROR("Shader compilation failed");
			return false;
		}
		IDxcBlobUtf8* errors;
		result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
		if (errors && errors->GetStringLength() != 0)
		{
			GE_CORE_ERROR("Shader compilation error: {0}", errors->GetStringPointer());
			return false;
		}

		IDxcBlob* compiledShaderBlob;
		hr = result->GetResult(&compiledShaderBlob);
		if (FAILED(hr))
		{
			GE_CORE_ERROR("Get compiled shader failed");
			return false;
		}
		output.resize(compiledShaderBlob->GetBufferSize());
		memcpy(output.data(), compiledShaderBlob->GetBufferPointer(), compiledShaderBlob->GetBufferSize());

		ID3D12ShaderReflection* reflection;
		IDxcBlob* reflectionBlob;
		hr = result->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflectionBlob), nullptr);
		if (FAILED(hr))
		{
			GE_CORE_ERROR("Get shader reflection failed");
		}
		DxcBuffer reflectionData;
		reflectionData.Encoding = DXC_CP_ACP;
		reflectionData.Ptr		= reflectionBlob->GetBufferPointer();
		reflectionData.Size		= reflectionBlob->GetBufferSize();
		m_Utils->CreateReflection(&reflectionData, IID_PPV_ARGS(&reflection));

		// use reflection
		reflection->

		return true;
	}
}