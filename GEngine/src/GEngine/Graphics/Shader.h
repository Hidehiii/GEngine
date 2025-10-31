#pragma once
#include "GEngine/Core/Core.h"
#include <unordered_map>
#include "GEngine/Math/Math.h"
#include "GEngine/Core/Buffer.h"
#include "GEngine/Graphics/Texture.h"
#include "GEngine/Compute/StorageImage.h"
#include "GEngine/Compute/StorageBuffer.h"
#include <algorithm>

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
		SHADER_PROPERTY_TYPE_NONE,
		SHADER_PROPERTY_TYPE_INT,
		SHADER_PROPERTY_TYPE_FLOAT,
		SHADER_PROPERTY_TYPE_VECTOR,
		SHADER_PROPERTY_TYPE_COLOR,

		SHADER_PROPERTY_TYPE_SAMPLER_2D,
		SHADER_PROPERTY_TYPE_SAMPLER_CUBE,
		SHADER_PROPERTY_TYPE_SAMPLER_2D_ARRAY,

		SHADER_PROPERTY_TYPE_SAMPLER,
		SHADER_PROPERTY_TYPE_TEXTURE_2D,
		SHADER_PROPERTY_TYPE_TEXTURE_CUBE,
		SHADER_PROPERTY_TYPE_TEXTURE_2D_ARRAY,

		SHADER_PROPERTY_TYPE_STORAGE_IMAGE_2D,
		SHADER_PROPERTY_TYPE_STORAGE_BUFFER,
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
		BLEND_FACTOR_SRC_ALPHA				= 1,
		BLEND_FACTOR_DST_ALPHA				= 2,
		BLEND_FACTOR_SRC_COLOR				= 3,
		BLEND_FACTOR_DST_COLOR				= 4,
		BLEND_FACTOR_ONE_MINUS_SRC_ALPHA	= 5,
		BLEND_FACTOR_ONE_MINUS_DST_ALPHA	= 6,
		BLEND_FACTOR_ONE_MINUS_SRC_COLOR	= 7,
		BLEND_FACTOR_ONE_MINUS_DST_COLOR	= 8,
		BLEND_FACTOR_ONE					= 9,
		BLEND_FACTOR_ZERO					= 10
	};
	enum CullMode
	{
		CULL_MODE_NONE	= 0,
		CULL_MODE_FRONT	= 1,
		CULL_MODE_BACK	= 2
	};
	enum BlendMode
	{
		BLEND_MODE_NONE					= 0,
		BLEND_MODE_ADD					= 1,
		BLEND_MODE_SUBSTRACT			= 2,
		BLEND_MODE_REVERSE_SUBSTRACT	= 3,
		BLEND_MODE_MIN					= 4,
		BLEND_MODE_MAX					= 5,
	};
	enum CompareOperation
	{
		COMPARE_OP_LESS				= 0,
		COMPARE_OP_GREATER			= 1,
		COMPARE_OP_LESS_EQUAL		= 2,
		COMPARE_OP_GREATER_EQUAL	= 3,
		COMPARE_OP_EQUAL			= 4,
		COMPARE_OP_NOT_EQUAL		= 5,
		COMPARE_OP_ALWAYS			= 6,
	};
	enum ColorMaskChannel
	{
		COLOR_MASK_CHANNLE_R = BIT(0),
		COLOR_MASK_CHANNLE_G = BIT(1),
		COLOR_MASK_CHANNLE_B = BIT(2),
		COLOR_MASK_CHANNLE_A = BIT(3),
	};

	struct RenderState
	{
		bool				DepthWrite					= true;
		CompareOperation	DepthTestOp					= COMPARE_OP_LESS_EQUAL;
		int					ColorMask					= COLOR_MASK_CHANNLE_R | COLOR_MASK_CHANNLE_G | COLOR_MASK_CHANNLE_B | COLOR_MASK_CHANNLE_A;
		BlendFactor			BlendColorSrc				= BLEND_FACTOR_ONE;
		BlendFactor			BlendColorDst				= BLEND_FACTOR_ZERO;
		BlendFactor			BlendAlphaSrc				= BLEND_FACTOR_ONE;
		BlendFactor			BlendAlphaDst				= BLEND_FACTOR_ZERO;
		BlendMode			BlendColor					= BLEND_MODE_NONE;
		BlendMode			BlendAlpha					= BLEND_MODE_NONE;
		CullMode			Cull						= CULL_MODE_BACK;
		std::string			Tag							= "Default";
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
		std::unordered_map<std::string, ConstShaderProperty>		ConstPropertiesDesc; // name : property
		Buffer														ConstProperties;
		std::unordered_map<std::string, ReferenceShaderProperty>	ReferenceProperties; // name : property
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
		virtual const std::unordered_map<std::string, ShaderPropertyType>&			GetProperties() { return m_Properties; }
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

		std::unordered_map<std::string, ShaderPropertyType>			m_Properties; // name : type
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