#pragma once
#include "GEngine/Core/Core.h"
#include <string>
#include <unordered_map>
#include "GEngine/Math/Math.h"
#include "GEngine/Core/Buffer.h"
#include "GEngine/Graphics/Texture.h"
#include "GEngine/Compute/StorageImage.h"
#include "GEngine/Compute/StorageBuffer.h"
#include <algorithm>
#include <filesystem>
#include "Material.h"
#include "ShaderCompiler.h"

#define SHADER_STAGE_VERTEX "vertex"
#define SHADER_STAGE_HULL "hull"
#define SHADER_STAGE_DOMAIN "domain"
#define SHADER_STAGE_GEOMETRY "geometry"
#define SHADER_STAGE_FRAGMENT "fragment"

#define SHADER_STAGE_AMPLIFICATION "amplification"
#define SHADER_STAGE_MESH "mesh"

#define SHADER_STAGE_COMPUTE "compute"


namespace GEngine
{
	enum ShaderPropertyType
	{
		None, 
		Int,
		Float,
		Vector,
		Color,

		Sampler2D,
		SamplerCub,
		Sampler2DArray,

		Sampler,
		Texture2D,
		TextureCube,
		Texture2DArray,

		StorageImage2D,
		StorageBuffer,
	};
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
	enum BlendFactor
	{
		SRC_ALPHA			= 1,
		DST_ALPHA			= 2,
		SRC_COLOR			= 3,
		DST_COLOR			= 4,
		ONE_MINUS_SRC_ALPHA = 5,
		ONE_MINUS_DST_ALPHA = 6,
		ONE_MINUS_SRC_COLOR = 7,
		ONE_MINUS_DST_COLOR = 8,
		ONE					= 9,
		ZERO				= 10
	};
	enum CullMode
	{
		None	= 0,
		Front	= 1,
		Back	= 2
	};
	enum BlendMode
	{
		None				= 0,
		Add					= 1,
		Substract			= 2,
		ReverseSubstract	= 3,
		Min					= 4,
		Max					= 5,
	};
	enum CompareOperation
	{
		Less			= 0,
		Greater			= 1,
		LessEqual		= 2,
		GreaterEqual	= 3,
		Equal			= 4,
		NotEqual		= 5,
		Always			= 6,
	};
	enum ColorMaskChannel
	{
		R = 1 < 0,
		G = 1 < 1,
		B = 1 < 2,
		A = 1 < 3,
	};

	struct RenderState
	{
		bool				DepthWrite					= true;
		CompareOperation	DepthTestOp					= CompareOperation::LessEqual;
		int					ColorMask					= ColorMaskChannel::R | ColorMaskChannel::G | ColorMaskChannel::B | ColorMaskChannel::A;
		BlendFactor			BlendColorSrc				= BlendFactor::ONE;
		BlendFactor			BlendColorDst				= BlendFactor::ZERO;
		BlendFactor			BlendAlphaSrc				= BlendFactor::ONE;
		BlendFactor			BlendAlphaDst				= BlendFactor::ZERO;
		BlendMode			BlendColor					= BlendMode::None;
		BlendMode			BlendAlpha					= BlendMode::None;
		CullMode			Cull						= CullMode::Back;
	};

	struct ConstShaderProperty
	{
		uint32_t			Size = 0;
		uint32_t			Location = 0;
	};

	struct ReferenceShaderProperty
	{
		uint32_t			Location = 0;
		void*				Ptr = nullptr;
	};

	struct ShaderPass
	{
		RenderState													State;
		std::unordered_map<std::string, ConstShaderProperty>		ConstPropertiesDesc;
		Buffer														ConstProperties;
		std::unordered_map<std::string, ReferenceShaderProperty>	ReferenceProperties;
	};

	namespace Utils
	{
		uint32_t GetShaderPropertyTypeSize(ShaderPropertyType type);
		std::string ToLower(std::string string);
		std::string ToUpper(std::string string);
		uint8_t ShaderVertexInputFlagToLocation(const std::string& flag);

		bool ShaderBoolFromString(const std::string& value);
		std::string ShaderTypeFromString(const std::string& type);
		CompareOperation ShaderCompareOperationFromString(const std::string& value);

		CullMode ShaderCullModeFromString(const std::string& value);

		std::vector<std::string> SplitString(const std::string& string, char delimiter);

		std::string RemoveCharFromString(const std::string& string, char character);

		std::string RemoveCharFromStringInHead(const std::string& string, char character);
		std::string RemoveCharFromStringInTail(const std::string& string, char character);


		ShaderPropertyType ShaderPropertyTypeFromString(const std::string& type);

		BlendFactor ShaderBlendFactorFromString(const std::string& factor);

		BlendMode ShaderBlendModeFromString(const std::string& type);
		const char* GetCacheDirectory();

		void CreateCacheDirectoryIfNeeded();

		std::string ReadFile(const std::string& path);
	}

	class GENGINE_API Shader
	{
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

		virtual const std::string&											GetShaderName() const { return m_Name; }
		virtual const std::string&											GetShaderPath() { return m_FilePath; }
		virtual const std::unordered_map<std::string, ShaderPropertyType>&	GetProperties() { return m_Properties; }
		virtual const std::vector<ShaderPass>&								GetPasses() { return m_Passes; }

	protected:
		virtual void										Preprocess(const std::string& source, std::vector<std::vector<std::string>>& passStages, std::vector<std::string>& shaderSrcCodes);
		virtual bool										Compile(std::vector<std::vector<std::string>>& passStages, std::vector<std::string>& shaderSrcCodes) = 0;
	protected:
		std::string											m_FilePath;
		std::string											m_Name;

		std::unordered_map<std::string, ShaderPropertyType>	m_Properties;
		std::vector<ShaderPass>								m_Passes;

	public:
		static Ref<Shader>									GetShader(const std::string& name);
		static Ref<Shader>									Create(const std::string& path);
	private:
		// name, shader
		static std::unordered_map<std::string, Ref<Shader>>	s_Shaders;
	};
}