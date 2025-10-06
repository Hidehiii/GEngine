#pragma once
#include "GEngine/Core/Core.h"

namespace GEngine
{
	enum ShaderCompilerType
	{
		Dxc		= 0,
		Shaderc	= 1,
	};

	class GENGINE_API ShaderCompiler
	{
	public:
		virtual bool Compile(const std::string& source, std::vector<LPWSTR>& args, std::vector<uint8_t>& output) = 0;

		static void Create(ShaderCompilerType type);

		static Ref<ShaderCompiler> Get() { return s_Instance; }

		static Ref<ShaderCompiler>	s_Instance;
	};
}


