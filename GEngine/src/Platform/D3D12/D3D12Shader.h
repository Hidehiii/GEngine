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
		struct RootBindingLayout
		{
			int ResourceTableRootIndex = -1;
			int SamplerTableRootIndex = -1;
		};

		D3D12Shader(const std::string& path);
		virtual ~D3D12Shader();

		bool operator==(const D3D12Shader& other) const
		{
			return m_FilePath == other.m_FilePath;
		}

	public:
		const std::vector<std::unordered_map<std::string, D3D12_SHADER_BYTECODE>>&	GetByteCodes() const { return m_ByteCodes; }
		const std::vector<Microsoft::WRL::ComPtr<ID3D12RootSignature>>&				GetRootSignatures() const { return m_RootSignatures; }
		const RootBindingLayout& GetRootBindingLayout(uint32_t pass) const { return m_RootBindingLayouts.at(pass); }

	private:
		virtual void ProcessMachineCode(const std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& shaders) override; //  pass { stage : byte}

		void CreateRootSignatures();
	private:
		std::vector<std::unordered_map<std::string, D3D12_SHADER_BYTECODE>>	m_ByteCodes; // pass { stage : bytecode }
		// D3D12_SHADER_BYTECODE stores a raw pointer; retain the source bytes for
		// the whole shader lifetime instead of pointing at InitializeShader locals.
		std::vector<std::unordered_map<std::string, std::vector<std::byte>>> m_CompiledByteCodes;
		std::vector<Microsoft::WRL::ComPtr<ID3D12RootSignature>>			m_RootSignatures;
		std::vector<RootBindingLayout>								m_RootBindingLayouts;
	};
}


