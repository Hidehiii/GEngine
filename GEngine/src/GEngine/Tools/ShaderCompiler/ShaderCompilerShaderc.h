#pragma once
#include "ShaderCompiler.h"
#include <shaderc/shaderc.hpp>
#include <SPIRVCross/spirv_cross.hpp>
#include <SPIRVCross/spirv_glsl.hpp>
#include <SPIRVCross/spirv_hlsl.hpp>

namespace GEngine
{
	class ShaderCompilerShaderc : public ShaderCompiler
	{
	public:
		ShaderCompilerShaderc();

		virtual bool Compile(const std::string& source, std::vector<LPWSTR>& args, std::vector<uint8_t>& output) override;
	};
}