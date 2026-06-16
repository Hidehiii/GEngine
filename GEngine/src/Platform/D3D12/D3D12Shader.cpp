#include "GEpch.h"
#include "D3D12Shader.h"


namespace GEngine
{
	D3D12Shader::D3D12Shader(const std::string& path)
	{
		InitializeShader(path, GE_BIND_CLASS_FUNCTION_LAMBDA(ProcessMachineCode));
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
}