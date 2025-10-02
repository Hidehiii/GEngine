#include "GEpch.h"
#include "ShaderCompiler.h"


namespace GEngine
{
	Ref<ShaderCompiler> ShaderCompiler::s_Instance = CreateRef<ShaderCompiler>();

	ShaderCompiler::ShaderCompiler()
	{
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(m_Utils.GetAddressOf()));
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(m_Compiler.GetAddressOf()));
		m_Utils->CreateDefaultIncludeHandler(m_IncludeHandler.GetAddressOf());
	}

	bool ShaderCompiler::Compile(const std::string& source, std::vector<LPWSTR>& args, std::vector<uint8_t>& output, std::vector<uint8_t>& reflection)
	{
		DxcBuffer				sourceBuffer;
		sourceBuffer.Ptr		= source.data();
		sourceBuffer.Size		= source.size();
		sourceBuffer.Encoding	= DXC_CP_ACP;

		Microsoft::WRL::ComPtr<IDxcResult> result;
		HRESULT hr = m_Compiler->Compile(&sourceBuffer, args.data(), (uint32_t)args.size(), m_IncludeHandler.Get(), IID_PPV_ARGS(result.GetAddressOf()));
		if (FAILED(hr))
		{
			GE_CORE_ERROR("Shader compilation failed");
			return false;
		}
		Microsoft::WRL::ComPtr<IDxcBlobUtf8> errors;
		result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(errors.GetAddressOf()), nullptr);
		if (errors && errors->GetStringLength() != 0)
		{
			GE_CORE_ERROR("Shader compilation error: {0}", errors->GetStringPointer());
			return false;
		}

		Microsoft::WRL::ComPtr<IDxcBlob> compiledShaderBlob;
		hr = result->GetResult(compiledShaderBlob.GetAddressOf());
		if (FAILED(hr))
		{
			GE_CORE_ERROR("Get compiled shader failed");
			return false;
		}
		output.resize(compiledShaderBlob->GetBufferSize());
		memcpy(output.data(), compiledShaderBlob->GetBufferPointer(), compiledShaderBlob->GetBufferSize());

		Microsoft::WRL::ComPtr<IDxcBlob> reflectionBlob;
		hr = result->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(reflectionBlob.GetAddressOf()), nullptr);
		if (FAILED(hr))
		{
			GE_CORE_ERROR("Get shader reflection failed");
		}
		reflection.resize(reflectionBlob->GetBufferSize());
		memcpy(reflection.data(), reflectionBlob->GetBufferPointer(), reflectionBlob->GetBufferSize());
		
		return true;
	}
}