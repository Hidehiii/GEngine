#pragma once
#include "GEngine/Core/Core.h"
#include <string>
#include <unordered_map>
#include "GEngine/Math/Math.h"
#include "GEngine/Core/Buffer.h"
#include "GEngine/Renderer/Texture.h"
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
	};
	class GENGINE_API ShaderMacroName
	{
	public:
		static const	std::string			GE_ATTACHMENT_UV_STARTS_AT_TOP;
		static const	std::string			GE_BINDING_START; 
		static const	std::string			GE_MATERIAL_DEFINE;
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

	namespace Utils
	{
		uint32_t ShaderUniformTypeSize(ShaderUniformType type);
		std::string ShaderUniformTypeToGlslType(ShaderUniformType type);
		std::string ToLower(std::string string);
		std::string ToUpper(std::string string);
		uint8_t ShaderVertexInputFlagToLocation(const std::string& flag);

		bool ShaderBoolFromString(const std::string& value);

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
	}

	class GENGINE_API Shader
	{
	public:
		virtual ~Shader() = default;

		virtual BlendMode									GetBlendModeColor() { return m_BlendModeColor; }
		virtual BlendMode									GetBlendModeAlpha() { return m_BlendModeAlpha; }
		virtual CullMode									GetCullMode() { return m_CullMode; }
		virtual BlendFactor									GetBlendColorSourceFactor() { return m_BlendColorSourceFactor; }
		virtual BlendFactor									GetBlendColorDestinationFactor() { return m_BlendColorDestinationFactor; }
		virtual BlendFactor									GetBlendAlphaSourceFactor() { return m_BlendAlphaSourceFactor; }
		virtual BlendFactor									GetBlendAlphaDestinationFactor() { return m_BlendAlphaDestinationFactor; }
		virtual bool										GetEnableDepthWrite()  { return m_EnableDepthWrite; }
		virtual CompareOperation							GetDepthTestOperation()  { return m_DepthTestOperation; }

		virtual std::vector<ShaderUniform>					GetUniforms()  { return m_UniformCache; }
		virtual const std::string&							GetShaderName() const  { return m_Name; }
		virtual void										SetShaderName(std::string name)  { m_Name = name; }
		virtual std::string									GetShaderPath() { return m_FilePath; }
		virtual std::vector<ShaderUniformTexture2D>			GetTexture2D()  { return m_Texture2DCache; }
		virtual uint32_t									GetTexture2DCount()  { return m_Texture2DCache.size(); }
		virtual std::vector<ShaderUniformStorageImage2D>	GetStorageImage2D()  { return m_StorageImage2DCache; }
		virtual std::vector<ShaderUniformStorageBuffer>		GetStorageBuffer() { return m_StorageBufferCache; }
		virtual std::vector<ShaderUniformCubeMap>			GetCubeMap() { return m_CubeMapCache; }

		virtual std::string									GetShaderMainFuncName() { return m_ShaderMainFuncName; }

		virtual std::vector<uint32_t>						GetVertexShaderSource() = 0;
		virtual std::vector<uint32_t>						GetFragmentShaderSource() = 0;

		static Ref<Shader>									Create(const std::string& path);
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
		const std::string									m_ShaderMainFuncName			= "main";
		
	};

	class GENGINE_API ShaderLibrary
	{
	public:
		static std::string Add(Ref<Shader>& shader);
		static Ref<Shader> Load(const std::string& path);
		static Ref<Shader> Get(const std::string& name);
		static Ref<Shader> GetShader(const std::string& name) { return Get(name);}
		static size_t Size() { return m_Shaders.size(); }
		static std::vector<std::string> GetShaderNames();
		static bool Exists(const std::string& name) { return m_Shaders.find(name) != m_Shaders.end(); }
	private:
		static std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};







	
}