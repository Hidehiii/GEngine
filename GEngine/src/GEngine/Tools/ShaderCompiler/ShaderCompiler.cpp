#include "GEpch.h"
#include "ShaderCompiler.h"
#include "ShaderCompilerDxc.h"
#include "ShaderCompilerShaderc.h"

namespace GEngine
{
	Ref<ShaderCompiler> ShaderCompiler::s_Instance = nullptr;

	void ShaderCompiler::Create(ShaderCompilerType type)
	{
		switch (type)
		{
		case Dxc:
			s_Instance = CreateRef<ShaderCompilerDxc>();
			break;
		case Shaderc:
			s_Instance = CreateRef<ShaderCompilerShaderc>();
			break;
		default:
			break;
		}
	}
}