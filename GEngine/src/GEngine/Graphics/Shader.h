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



namespace GEngine
{
	enum class ShaderUniformType
	{
		None			= 0, 
		Int				= 1,
		Float			= 2,
		Vector			= 3,
		Color			= 4,
		Mat3			= 5,
		Mat4			= 6,
		Sampler2D		= 7,
		SamplerCube		= 8,
		StorageImage2D	= 9,
		StorageBuffer	= 10,
		Texture2D		= 11,
		Sampler			= 12,
		Sampler2DArray	= 13,
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
	enum class GENGINE_API BlendFactor
	{
		SRC_ALPHA		= 1,
		DST_ALPHA		= 2,
		SRC_COLOR		= 3,
		DST_COLOR		= 4,
		ONE_MINUS_SRC_ALPHA = 5,
		ONE_MINUS_DST_ALPHA = 6,
		ONE_MINUS_SRC_COLOR = 7,
		ONE_MINUS_DST_COLOR = 8,
		ONE = 9,
		ZERO = 10
	};
	enum class CullMode
	{
		None = 0,
		Front = 1,
		Back = 2
	};
	enum class BlendMode
	{
		None = 0,
		Add = 1,
		Substract = 2,
		ReverseSubstract = 3,
		Min = 4,
		Max = 5,
	};
	enum class CompareOperation
	{
		Less = 0,
		Greater = 1,
		LessEqual = 2,
		GreaterEqual = 3,
		Equal = 4,
		NotEqual = 5,
		Always = 6,
	};
	class GENGINE_API ShaderDataFlag
	{
	public:
		static const	std::string			None;
		static const	std::string			Name;
		static const	std::string			Blend;
		static const	std::string			DepthMask;
		static const	std::string			DepthTest;
		static const	std::string			Properties;
		static const	std::string			Type;
	};
	class GENGINE_API ShaderStage
	{
	public:
		static	const	std::string         Vertex;
		static	const	std::string         Fragment;
		static	const	std::string         Pixel;
		static	const	std::string			Compute;
		static	const	std::string			TessellationControl;
		static	const	std::string			TessellationEvaluation;
		static	const	std::string			Geometry;
	};
	
	struct ShaderUniform
	{
		ShaderUniform() = default;
		ShaderUniform(const ShaderUniform&) = default;

		std::string			Name = "";
		ShaderUniformType	Type = ShaderUniformType::None;
		uint32_t			Size = 0;
		uint32_t			Location = 0;
	};

	struct ShaderUniformTexture2D
	{
		std::string		Name;
		uint32_t		Slot;
		Ref<Texture2D>	Texture;
	};


	struct ShaderUniformStorageImage2D
	{
		std::string			Name;
		uint32_t			Slot;
		Ref<StorageImage2D> Image;
	};

	struct ShaderUniformStorageBuffer
	{
		std::string			Name;
		uint32_t			Slot;
		Ref<StorageBuffer>	Buffer;
	};


	struct ShaderUniformCubeMap
	{
		std::string		Name;
		uint32_t		Slot;
		Ref<CubeMap>	Cubemap;
	};

	struct ShaderUniformTexture2DArray
	{
		std::string				Name;
		uint32_t				Slot;
		Ref<Texture2DArray>		TextureArray;
	};

	struct RenderState
	{
		bool				DepthWrite					= true;
		CompareOperation	DepthTestOp					= CompareOperation::LessEqual;
		bool				ColorWrite					= true;
		BlendFactor			BlendColorSrc				= BlendFactor::ONE;
		BlendFactor			BlendColorDst				= BlendFactor::ZERO;
		BlendFactor			BlendAlphaSrc				= BlendFactor::ONE;
		BlendFactor			BlendAlphaDst				= BlendFactor::ZERO;
		BlendMode			BlendColor					= BlendMode::None;
		BlendMode			BlendAlpha					= BlendMode::None;
		CullMode			Cull						= CullMode::Back;
	};

	struct ShaderPass
	{
		RenderState										State;
		std::unordered_map<std::string, std::string>	Stages;
	};

	namespace Utils
	{
		uint32_t ShaderUniformTypeSize(ShaderUniformType type);
		std::string ShaderUniformTypeToGlslType(ShaderUniformType type);
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


		ShaderUniformType ShaderUniformTypeFromString(const std::string& type);

		BlendFactor ShaderBlendFactorFromString(const std::string& factor);

		BlendMode ShaderBlendModeFromString(const std::string& type);
		const char* GetCacheDirectory();

		void CreateCacheDirectoryIfNeeded();

		void SetShaderMacroBool(std::string& source, const std::string& macro, bool value);

		void SetShaderMacroExpression(std::string& source, const std::string& macro, std::string& exp);
		void InsertShaderVertexInput(std::string& source, const std::string& vertexInput);
		std::string ProcessShaderName(const std::string& source);
		void ProcessShaderBlend(const std::string& source, BlendMode& modeColor, BlendMode modeAlpha,
			BlendFactor& srcColor, BlendFactor& dstColor, BlendFactor& srcAlpha, BlendFactor& dstAlpha);
		void ProcessShaderDepthWrite(const std::string& source, bool& enableDepthWrite);
		void ProcessShaderDepthTest(const std::string& source, CompareOperation& depthTestOperation);
		void ProcessShaderCull(const std::string& source, CullMode& mode);
		std::string ProcessShaderVertexInput(const std::string& source);

		std::string ReadFile(const std::string& path);

		void ProcessShaderProperties(const std::string& source, std::vector<ShaderUniform>& uniformCache, std::vector<ShaderUniformTexture2D>& texture2DCache,
			std::vector<ShaderUniformCubeMap>& cubeMapCache, std::vector<ShaderUniformStorageImage2D>& storageImage2DCache,
			std::vector<ShaderUniformStorageBuffer>& storageBufferCache, uint32_t locationStart, uint32_t slotStart, uint32_t storageBufferSlotOffset);
		void ProcessShaderBlocks(const std::string& source, std::unordered_map<std::string, std::string>& blocks);
		void ProcessShaderPasses(const std::string& source, std::unordered_map<std::string, std::string>& blocks, std::unordered_map<std::string, ShaderPass>& passes);
		bool ProcessShaderStage(const std::string& source, const std::string& stage, std::string& blockName);
	}

	class GENGINE_API Shader
	{
	public:
		virtual ~Shader() = default;

		virtual BlendMode									GetBlendColor() { return m_BlendModeColor; }
		virtual BlendMode									GetBlendAlpha() { return m_BlendModeAlpha; }
		virtual CullMode									GetCull() { return m_CullMode; }
		virtual BlendFactor									GetBlendColorSrc() { return m_BlendColorSourceFactor; }
		virtual BlendFactor									GetBlendColorDst() { return m_BlendColorDestinationFactor; }
		virtual BlendFactor									GetBlendAlphaSrc() { return m_BlendAlphaSourceFactor; }
		virtual BlendFactor									GetBlendAlphaDst() { return m_BlendAlphaDestinationFactor; }
		virtual bool										IsEnableDepthWrite()  { return m_EnableDepthWrite; }
		virtual CompareOperation							GetDepthTestOp()  { return m_DepthTestOperation; }

		virtual std::vector<ShaderUniform>					GetUniforms()  { return m_UniformCache; }
		virtual const std::string&							GetShaderName() const  { return m_Name; }
		virtual void										SetShaderName(std::string name)  { m_Name = name; }
		virtual std::string									GetShaderPath() { return m_FilePath; }
		virtual std::vector<ShaderUniformTexture2D>			GetTexture2D()  { return m_Texture2DCache; }
		virtual uint32_t									GetTexture2DCount()  { return m_Texture2DCache.size(); }
		virtual std::vector<ShaderUniformStorageImage2D>	GetStorageImage2D()  { return m_StorageImage2DCache; }
		virtual std::vector<ShaderUniformStorageBuffer>		GetStorageBuffer() { return m_StorageBufferCache; }
		virtual std::vector<ShaderUniformCubeMap>			GetCubeMap() { return m_CubeMapCache; }
		virtual std::vector<ShaderUniformTexture2DArray>	GetTexture2DArray() { return m_Texture2DArrayCache; }

		virtual std::string									GetShaderMainFuncName() { return m_ShaderMainFuncName; }

		virtual std::vector<uint32_t>						GetVertexShaderSource() = 0;
		virtual std::vector<uint32_t>						GetFragmentShaderSource() = 0;

		
	protected:
		virtual void										Preprocess(std::string& source);

		virtual void										SetMacroBool(std::string& source) = 0;
		virtual void										SetMacroExp(std::string& source) = 0;

		virtual void										SetGlslVersionString(std::string& source);
		virtual void										SetMacroMaterialDefine(std::string& source);
	protected:
		// slot 从40开始
		static const uint32_t s_SlotOffset = 40;
	protected:
		std::string											m_FilePath;
		std::string											m_Name;
		std::vector<ShaderUniform>							m_UniformCache;
		std::vector<ShaderUniformTexture2D>					m_Texture2DCache;
		std::vector<ShaderUniformStorageImage2D>			m_StorageImage2DCache;
		std::vector<ShaderUniformStorageBuffer>				m_StorageBufferCache;
		std::vector<ShaderUniformCubeMap>					m_CubeMapCache;
		std::vector<ShaderUniformTexture2DArray>			m_Texture2DArrayCache;
		bool												m_EnableDepthWrite				= true;
		CompareOperation									m_DepthTestOperation			= CompareOperation::LessEqual;
		bool												m_EnableColorWrite				= true;
		BlendFactor											m_BlendColorSourceFactor		= BlendFactor::ONE;
		BlendFactor											m_BlendColorDestinationFactor	= BlendFactor::ZERO;
		BlendFactor											m_BlendAlphaSourceFactor		= BlendFactor::ONE;
		BlendFactor											m_BlendAlphaDestinationFactor	= BlendFactor::ZERO;
		BlendMode											m_BlendModeColor				= BlendMode::None;
		BlendMode											m_BlendModeAlpha				= BlendMode::None;
		CullMode											m_CullMode						= CullMode::Back;
		std::unordered_map<std::string, std::string>		m_ShaderBlocks;
		std::unordered_map<std::string, ShaderPass>			m_ShaderPasses;
		const std::string									m_ShaderMainFuncName			= "main";
		
	public:
		static Ref<Shader>									GetShader(const std::string& name);
		static Ref<Shader>									Create(const std::string& path);
	private:
		// name, shader
		static std::unordered_map<std::string, Ref<Shader>>	s_Shaders;
	};
}