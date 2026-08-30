#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsCommon.h"
#include <Unknwn.h>
#include "dxcapi.h"

namespace GEngine
{
	

	class GENGINE_API ShaderCompiler
	{
	public:
		ShaderCompiler();

		bool Compile(const std::string& source, const std::string& target, const std::string& entryPoint, 
					std::vector<std::byte>& machineCode, std::vector<std::byte>& reflectionDxil);

		static Ref<ShaderCompiler> Create();
		static Ref<ShaderCompiler> Get();

		void ReflectDxil(const std::vector<std::byte>& reflectionDxil, const std::string& target, ShaderReflectionInfo& reflectionOutput);
		// reflect spirv need dxil reflection meta, because spirv-cross can't reflect all the information we need
		void ReflectSpirv(const std::vector<std::byte>& spirvMachineCode, const std::vector<std::byte>& reflectionDxil, 
							const std::string& target, ShaderReflectionInfo& reflectionOutput);
	protected:
		bool CompileSpirv(const std::string& source, const std::string& target, const std::string& entryPoint,
							std::vector<std::byte>& machineCode);
		bool CompileDxil(const std::string& source, const std::string& target, const std::string& entryPoint,
			std::vector<std::byte>& machineCode, std::vector<std::byte>& reflectionDxil);
	private:
		IDxcUtils*					m_Utils;
		IDxcCompiler3*				m_Compiler;
		IDxcIncludeHandler*			m_IncludeHandler;

		static Ref<ShaderCompiler>	s_Instance;
	};
}
