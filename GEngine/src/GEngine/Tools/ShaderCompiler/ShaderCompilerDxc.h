#pragma once
#include "ShaderCompiler.h"
#include "dxcapi.h"

namespace GEngine
{
	class GENGINE_API ShaderCompilerDxc : public ShaderCompiler
	{
	public:
		ShaderCompilerDxc();

		bool Compile(const std::string& source, std::vector<LPWSTR>& args, std::vector<uint8_t>& output) override;
	private:
		IDxcUtils*			m_Utils;
		IDxcCompiler3*		m_Compiler;
		IDxcIncludeHandler*	m_IncludeHandler;

		static Ref<ShaderCompiler>					s_Instance;
	};
}
