#include "GEpch.h"
#include "ShaderCompiler.h"
#include "GEngine/Tools/StringHelper.h"
#include "GEngine/Graphics/Shader.h"
#include "GEngine/Graphics/Graphics.h"
#include <d3d12shader.h>

namespace GEngine
{
	Ref<ShaderCompiler> ShaderCompiler::s_Instance = nullptr;
	
	namespace Utils
	{
		const std::string DXShaderTargetFromString(const std::string& target)
		{
			if (target == SHADER_STAGE_VERTEX)					return "vs_6_6";
			if (target == SHADER_STAGE_FRAGMENT)				return "ps_6_6";
			if (target == SHADER_STAGE_COMPUTE)					return "cs_6_6";
			if (target == SHADER_STAGE_HULL)					return "hs_6_6";
			if (target == SHADER_STAGE_DOMAIN)					return "ds_6_6";
			if (target == SHADER_STAGE_GEOMETRY)				return "gs_6_6";
			if (target == SHADER_STAGE_AMPLIFICATION)			return "as_6_6";
			if (target == SHADER_STAGE_MESH)					return "ms_6_6";
			GE_CORE_ASSERT(false, "Unknown shader target!");
			return "";
		}
	}

	ShaderCompiler::ShaderCompiler()
	{
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_Utils));
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_Compiler));
		m_Utils->CreateDefaultIncludeHandler(&m_IncludeHandler);
	}

	bool ShaderCompiler::Compile(const std::string& source, const std::string& target, const std::string& entryPoint, std::vector<uint32_t>& machineCode)
	{
		DxcBuffer				sourceBuffer;
		sourceBuffer.Ptr		= source.data();
		sourceBuffer.Size		= source.size();
		sourceBuffer.Encoding	= DXC_CP_ACP;

		std::vector<LPCWSTR> args;
		if (Graphics::GetGraphicsAPI() == GraphicsAPI::GRAPHICS_API_Vulkan || Graphics::GetGraphicsAPI() == GraphicsAPI::GRAPHICS_API_OpenGL)
		{
			args.push_back(L"-spirv");
		}
		args.push_back(L"-E");
		args.push_back(StringHelper::StringToWideString(entryPoint).c_str());
		args.push_back(L"-T");
		args.push_back(StringHelper::StringToWideString(Utils::DXShaderTargetFromString(target)).c_str());
#if GE_DEBUG
		args.push_back(L"Zi");
#endif
		// ?? what to do 
		args.push_back(L"-Qstrip_reflect");

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
		machineCode.resize(compiledShaderBlob->GetBufferSize());
		memcpy(machineCode.data(), compiledShaderBlob->GetBufferPointer(), compiledShaderBlob->GetBufferSize());

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
		D3D12_SHADER_DESC desc;
		reflection->GetDesc(&desc);
		GE_CORE_TRACE("Input parameters:");
		for (int i = 0; i < desc.InputParameters; i++)
		{
			D3D12_SIGNATURE_PARAMETER_DESC inputDesc;
			reflection->GetInputParameterDesc(i, &inputDesc);

			GE_CORE_TRACE("Semantic name {}, index {}, register {}, SVtype {}, type {}!", inputDesc.SemanticName, inputDesc.SemanticIndex, inputDesc.Register, inputDesc.SystemValueType, inputDesc.ComponentType);
		}
		GE_CORE_TRACE("Resource binding:");
		for (int i = 0; i < desc.BoundResources; i++)
		{
			D3D12_SHADER_INPUT_BIND_DESC resDesc;
			reflection->GetResourceBindingDesc(i, &resDesc);

			GE_CORE_TRACE("var name {}, type {}, bind point {}, bind count {}, register space {}!", resDesc.Name, resDesc.Type, resDesc.BindPoint, resDesc.BindCount, resDesc.Space);
		}
		GE_CORE_TRACE("Output parameters:");
		for(int i = 0; i < desc.OutputParameters;i++)
		{
			D3D12_SIGNATURE_PARAMETER_DESC outputDesc;
			reflection->GetOutputParameterDesc(i, &outputDesc);

			GE_CORE_TRACE("Semantic name {}, index {}, register {}, SVtype {}, type {}!", outputDesc.SemanticName, outputDesc.SemanticIndex, outputDesc.Register, outputDesc.SystemValueType, outputDesc.ComponentType);
		}

		return true;
	}
	Ref<ShaderCompiler> ShaderCompiler::Create()
	{
		s_Instance = CreateRef<ShaderCompiler>();
		return s_Instance;
	}
	Ref<ShaderCompiler> ShaderCompiler::Get()
	{
		return s_Instance;
	}
}