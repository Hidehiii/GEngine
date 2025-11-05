#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsCommon.h"
#include <unordered_map>
#include "GEngine/Math/Math.h"
#include "GEngine/Core/Buffer.h"
#include "GEngine/Graphics/Texture.h"
#include "GEngine/Compute/StorageImage.h"
#include "GEngine/Compute/StorageBuffer.h"
#include <algorithm>




namespace GEngine
{
	
	class GENGINE_API ShaderMacroName
	{
	public:
		static const	std::string			GE_ATTACHMENT_UV_STARTS_AT_TOP;
		static const	std::string			GE_BINDING_START; 
		static const	std::string			GE_MATERIAL_DEFINE;
		static const	std::string			GE_GRAPHICS_API_OPENGL;
		static const	std::string			GE_GRAPHICS_API_VULKAN;
		static const	std::string			GE_GRAPHICS_API_D3D12;
		static const	std::string			GE_GRAPHICS_API;
	};
	
	

	namespace Utils
	{
		uint32_t GetShaderPropertyTypeSize(ShaderPropertyType type);
		

		CompareOperation ShaderCompareOperationFromString(const std::string& value);

		CullMode ShaderCullModeFromString(const std::string& value);


		ShaderPropertyType ShaderPropertyTypeFromString(const std::string& type);

		BlendFactor ShaderBlendFactorFromString(const std::string& factor);

		BlendMode ShaderBlendModeFromString(const std::string& type);
	}

	class GENGINE_API Shader
	{
	protected:
		Shader(const std::string& path, std::function<void(const std::vector<std::unordered_map<std::string, std::vector<uint32_t>>>&)> processMachingCodeFunc);
	public:
		virtual ~Shader() = default;

		virtual BlendMode									GetBlendColor(const int& pass) { return m_Passes.at(pass).State.BlendColor; }
		virtual BlendMode									GetBlendAlpha(const int& pass) { return m_Passes.at(pass).State.BlendAlpha; }
		virtual CullMode									GetCull(const int& pass) { return m_Passes.at(pass).State.Cull; }
		virtual BlendFactor									GetBlendColorSrc(const int& pass) { return m_Passes.at(pass).State.BlendColorSrc; }
		virtual BlendFactor									GetBlendColorDst(const int& pass) { return m_Passes.at(pass).State.BlendColorDst; }
		virtual BlendFactor									GetBlendAlphaSrc(const int& pass) { return m_Passes.at(pass).State.BlendAlphaSrc; }
		virtual BlendFactor									GetBlendAlphaDst(const int& pass) { return m_Passes.at(pass).State.BlendAlphaDst; }
		virtual bool										GetEnableDepthWrite(const int& pass) { return m_Passes.at(pass).State.DepthWrite; }
		virtual CompareOperation							GetDepthTestOp(const int& pass) { return m_Passes.at(pass).State.DepthTestOp; }
		virtual int											GetColorMask(const int& pass) { return m_Passes.at(pass).State.ColorMask; }

		virtual const std::string&													GetShaderName() const { return m_Name; }
		virtual const std::string&													GetShaderPath() { return m_FilePath; }
		virtual const std::vector<ShaderPass>&										GetPasses() { return m_Passes; }
		virtual const std::vector<std::unordered_map<std::string, std::string>>&	GetStageEntryPoints() { return m_StageEntryPoints; }
		virtual const std::unordered_map<std::string, std::string>&					GetStageEntryPoints(const int& pass) { return m_StageEntryPoints.at(pass); }

	protected:
		virtual void										Preprocess(const std::string& source, std::vector<std::string>& shaderSrcCodes);
		virtual bool										Compile(const std::vector<std::string>& shaderSrcCodes, std::vector<std::unordered_map<std::string, std::vector<uint32_t>>>& shaders);
		virtual void 										ProcessMachineCode(const std::vector<std::unordered_map<std::string, std::vector<uint32_t>>>& shaders) = 0;
	protected:
		std::string													m_FilePath;
		std::string													m_Name;

		std::vector<std::unordered_map<std::string, std::string>>	m_StageEntryPoints; // stage : name
		std::vector<ShaderPass>										m_Passes;

	public:
		static Ref<Shader>									GetShader(const std::string& name);
		static Ref<Shader>									Create(const std::string& path);
	private:
		// name, shader
		static std::unordered_map<std::string, Ref<Shader>>	s_Shaders;
	};
}