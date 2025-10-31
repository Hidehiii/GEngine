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
	protected:
		void ReflectHlsl(IDxcResult* result, const std::string& target);
		void ReflectSpirv(const std::vector<uint32_t>& spirvCode, const std::string& target);
	private:
		IDxcUtils*					m_Utils;
		IDxcCompiler3*				m_Compiler;
		IDxcIncludeHandler*			m_IncludeHandler;

		static Ref<ShaderCompiler>	s_Instance;
	};
}
