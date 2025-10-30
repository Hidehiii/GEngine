#pragma once
#include "GEngine/Core/Core.h"
#include <Unknwn.h>
#include "dxcapi.h"

namespace GEngine
{
	class GENGINE_API ShaderCompiler
	{
	public:
		ShaderCompiler();

		bool Compile(const std::string& source, const std::string& target, const std::string& entryPoint, std::vector<uint32_t>& machineCode);

		static Ref<ShaderCompiler> Create();
		static Ref<ShaderCompiler> Get();
	private:
		IDxcUtils*					m_Utils;
		IDxcCompiler3*				m_Compiler;
		IDxcIncludeHandler*			m_IncludeHandler;

		static Ref<ShaderCompiler>	s_Instance;
	};
}
