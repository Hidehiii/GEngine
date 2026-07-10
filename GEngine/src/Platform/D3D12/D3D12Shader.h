#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/Shader.h"
#include <windows.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <directx/d3dx12.h>

namespace GEngine
{
	class GENGINE_API D3D12Shader : public Shader
	{
	public:
		D3D12Shader(const std::string& path);
		virtual ~D3D12Shader();

		bool operator==(const D3D12Shader& other) const
		{
			return m_ByteCodes == other.m_ByteCodes;
		}
	private:
		virtual void ProcessMachineCode(const std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& shaders) override; //  pass { stage : byte}

		void CreateRootSignatures();
	private:
		std::vector<std::unordered_map<std::string, D3D12_SHADER_BYTECODE>>	m_ByteCodes; // pass { stage : bytecode }
		std::vector<Microsoft::WRL::ComPtr<ID3D12RootSignature>>			m_RootSignatures;
	};
}


