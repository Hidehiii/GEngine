#pragma once
#include "GEngine/Core/Core.h"
#include "dxcapi.h"
#include <Windows.h>
#include <wrl.h>

namespace GEngine
{
	class GENGINE_API ShaderCompiler
	{
	public:
		ShaderCompiler();

		bool Compile(const std::string& source, std::vector<LPWSTR>& args, std::vector<uint8_t>& output, std::vector<uint8_t>& reflection);

		static Ref<ShaderCompiler> Get() { return s_Instance; }
	private:
		Microsoft::WRL::ComPtr<IDxcUtils>			m_Utils;
		Microsoft::WRL::ComPtr<IDxcCompiler3>		m_Compiler;
		Microsoft::WRL::ComPtr<IDxcIncludeHandler>	m_IncludeHandler;

		static Ref<ShaderCompiler>					s_Instance;
	};
}


