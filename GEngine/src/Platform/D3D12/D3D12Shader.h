#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/Shader.h"
#include <d3d12.h>
#include <dxgi1_6.h>

namespace GEngine
{
	class GENGINE_API D3D12Shader : public Shader
	{
	public:
		D3D12Shader(const std::string& path);
		virtual ~D3D12Shader();

	private:
		virtual void ProcessMachineCode(const std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& shaders) override; //  pass { stage : byte}
	private:
		std::vector<std::unordered_map<std::string, D3D12_SHADER_BYTECODE>>	m_ByteCodes; // pass { stage : bytecode }
	};
}


