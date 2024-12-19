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
		StorageBuffer	= 10
	};
	class GENGINE_API ShaderMacroName
	{
	public:
		static std::string			GE_ATTACHMENT_UV_STARTS_AT_TOP;
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
		static std::string			None;
		static std::string			Name;
		static std::string			Blend;
		static std::string			DepthMask;
		static std::string			DepthTest;
		static std::string			Properties;
		static std::string			Type;
	};
	class GENGINE_API ShaderStage
	{
	public:
		static	std::string         Vertex;
		static	std::string         Fragment;
		static	std::string         Pixel;
	};
	static uint32_t ShaderUniformTypeSize(ShaderUniformType type)
	{
		switch (type)
		{
		case ShaderUniformType::Int:	return 4;
		case ShaderUniformType::Float:	return 4;
		case ShaderUniformType::Vector: return 4 * 4;
		case ShaderUniformType::Color:	return 4 * 4;
		case ShaderUniformType::Mat3:	return 4 * 4 * 3;
		case ShaderUniformType::Mat4:	return 4 * 4 * 4;
		}
		return 0;
	}
	
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
		static std::string ToLower(std::string string)
		{
			std::transform(string.begin(), string.end(), string.begin(), ::tolower);
			return string;
		}
		static std::string ToUpper(std::string string)
		{
			std::transform(string.begin(), string.end(), string.begin(), ::toupper);
			return string;
		}

		static uint8_t ShaderVertexInputFlagToLocation(const std::string& flag)
		{
			if (ToLower(flag) == "position")	return 0;
			if (ToLower(flag) == "color")		return 1;
			if (ToLower(flag) == "normal")		return 2;
			if (ToLower(flag) == "tangent")		return 3;
			if (ToLower(flag) == "uv0")			return 4;
			if (ToLower(flag) == "uv1")			return 5;
			if (ToLower(flag) == "uv2")			return 6;
			if (ToLower(flag) == "uv3")			return 7;
			GE_CORE_ASSERT(false, "invalid vertex input");
		}

		static bool ShaderBoolFromString(const std::string& value)
		{
			if (ToLower(value) == "on")				return true;
			if (ToLower(value) == "1")				return true;
			if (ToLower(value) == "true")			return true;
			return false;
		}

		static CompareOperation ShaderCompareOperationFromString(const std::string& value)
		{
			if (ToLower(value) == "less")				return CompareOperation::Less;
			if (ToLower(value) == "greater")			return CompareOperation::Greater;
			if (ToLower(value) == "lessequal")			return CompareOperation::LessEqual;
			if (ToLower(value) == "lequal")				return CompareOperation::LessEqual;
			if (ToLower(value) == "greaterequal")		return CompareOperation::GreaterEqual;
			if (ToLower(value) == "gequal")				return CompareOperation::GreaterEqual;
			if (ToLower(value) == "equal")				return CompareOperation::Equal;
			if (ToLower(value) == "notequal")			return CompareOperation::NotEqual;
			if (ToLower(value) == "always")				return CompareOperation::Always;

			return CompareOperation::LessEqual;
		}

		static CullMode ShaderCullModeFromString(const std::string& value)
		{
			if (ToLower(value) == "none") return CullMode::None;
			if (ToLower(value) == "back") return CullMode::Back;
			if (ToLower(value) == "front") return CullMode::Front;
			return CullMode::Back;
		}

		static std::vector<std::string> SplitString(const std::string& string, char delimiter)
		{
			std::vector<std::string> result;
			std::stringstream ss(string);
			std::string item;
			while (std::getline(ss, item, delimiter))
			{
				item = item.substr(0, item.size());
				if (item.empty() == false)
					result.push_back(item);
			}
			return result;
		}

		static std::string RemoveCharFromString(const std::string& string, char character)
		{
			std::string result = string;
			for (auto it = result.begin(); it != result.end();)
			{
				if (*it == character)
					it = result.erase(it);
				else
					++it;
			}
			return result;
		}

		static std::string RemoveCharFromStringInHead(const std::string& string, char character)
		{
			std::string result = string;
			for (auto it = result.begin(); it != result.end();)
			{
				if (*it == character)
					it = result.erase(it);
				else
					return result;
			}
			return result;
		}
		static std::string RemoveCharFromStringInTail(const std::string& string, char character)
		{
			std::string result = string;
			for (int i = result.size() - 1; i >= 0; i--)
			{
				if (result.at(i) == character)
					result.erase(i);
				else
					return result;
			}
			return result;
		}


		static ShaderUniformType ShaderUniformTypeFromString(const std::string& type)
		{
			if (ToLower(type) == "int")				return ShaderUniformType::Int;
			if (ToLower(type) == "float")			return ShaderUniformType::Float;
			if (ToLower(type) == "vector")			return ShaderUniformType::Vector;
			if (ToLower(type) == "color")			return ShaderUniformType::Color;
			if (ToLower(type) == "mat3")			return ShaderUniformType::Mat3;
			if (ToLower(type) == "mat4")			return ShaderUniformType::Mat4;
			if (ToLower(type) == "sampler2d")		return ShaderUniformType::Sampler2D;
			if (ToLower(type) == "samplercube")		return ShaderUniformType::SamplerCube;
			if (ToLower(type) == "storageimage2d")	return ShaderUniformType::StorageImage2D;
			if (ToLower(type) == "storagebuffer")	return ShaderUniformType::StorageBuffer;
			GE_CORE_ASSERT(false, "Unknown shader uniform type! " + type);
			return ShaderUniformType::None;
		}

		static BlendFactor ShaderBlendFactorFromString(const std::string& factor)
		{
			if (ToUpper(factor) == "SRCALPHA")			return BlendFactor::SRC_ALPHA;
			if (ToUpper(factor) == "DSTALPHA")			return BlendFactor::DST_ALPHA;
			if (ToUpper(factor) == "SRCCOLOR")			return BlendFactor::SRC_COLOR;
			if (ToUpper(factor) == "DSTCOLOR")			return BlendFactor::DST_COLOR;
			if (ToUpper(factor) == "ONEMINUSSRCALPHA")	return BlendFactor::ONE_MINUS_SRC_ALPHA;
			if (ToUpper(factor) == "ONEMINUSDSTALPHA")	return BlendFactor::ONE_MINUS_DST_ALPHA;
			if (ToUpper(factor) == "ONEMINUSSRCCOLOR")	return BlendFactor::ONE_MINUS_SRC_COLOR;
			if (ToUpper(factor) == "ONEMINUSDSTCOLOR")	return BlendFactor::ONE_MINUS_DST_COLOR;
			if (ToUpper(factor) == "ONE")				return BlendFactor::ONE;
			if (ToUpper(factor) == "ZERO")				return BlendFactor::ZERO;

			GE_CORE_ASSERT(false, "Unknown blend factor! " + factor);
		}

		static BlendMode ShaderBlendModeFromString(const std::string& type)
		{
			if (ToLower(type) == "none")			return BlendMode::None;
			if (ToLower(type) == "add")				return BlendMode::Add;
			if (ToLower(type) == "substract")		return BlendMode::Substract;
			if (ToLower(type) == "reversesubstract")		return BlendMode::ReverseSubstract;
			if (ToLower(type) == "min")		return BlendMode::Min;
			if (ToLower(type) == "max")		return BlendMode::Max;

			GE_CORE_ASSERT(false, "Unknown blend type! " + type);
			return BlendMode::None;
		}
		static const char* GetCacheDirectory()
		{
			// TODO: make sure the assets directory is valid
			return "Assets/Cache/Shaders";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			std::string cacheDirectory = GetCacheDirectory();
			if (!std::filesystem::exists(cacheDirectory))
				std::filesystem::create_directories(cacheDirectory);
		}

		static void SetShaderMacroBool(std::string& source, const std::string& macro, bool value)
		{
			//第二行插入
			size_t eol = source.find_first_of("\n", 0);
			source.insert(eol + 1, "#define " + macro + " " + (value ? "1" : "0") + "\n");
		}

		static void SetShaderMacroExpression(std::string& source, const std::string& macro, std::string& exp)
		{
			//第二行插入
			size_t eol = source.find_first_of("\n", 0);
			source.insert(eol + 1, "#define " + macro + " " + exp + "\n");
		}
		static void InsertShaderVertexInput(std::string& source, const std::string& vertexInput)
		{
			//第二行插入
			size_t eol = source.find_first_of("\n", 0);
			source.insert(eol + 1, vertexInput);
		}
		static std::string ProcessShaderName(const std::string& source)
		{
			std::string output;

			const char* token = "#Name";
			size_t tokenLength = strlen(token);
			size_t pos = source.find(token, 0);
			if (pos == std::string::npos)
			{
				return output;
			}
			size_t eol = source.find_first_of("\r\n", pos);
			GE_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + tokenLength + 1;
			std::string name = source.substr(begin, eol - begin);
			int index = 0;
			while ((index = name.find(' ', index)) != std::string::npos)
			{
				name.erase(index, 1);
			}
			output = name;
			GE_CORE_TRACE("Shader name: {0}", name);
			return output;
		}
		static void ProcessShaderBlend(const std::string& source, BlendMode& modeColor, BlendMode modeAlpha, BlendFactor& srcColor, BlendFactor& dstColor, BlendFactor& srcAlpha, BlendFactor& dstAlpha)
		{
			const char* token = "#Blend";
			size_t tokenLength = strlen(token);
			size_t pos = source.find(token, 0);
			if (pos == std::string::npos)
			{
				return;
			}
			size_t eol = source.find_first_of("\r\n", pos);
			GE_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + tokenLength + 1;
			std::string blendString = source.substr(begin, eol - begin);
			blendString = Utils::RemoveCharFromString(blendString, ';');
			blendString = Utils::RemoveCharFromString(blendString, '\r');
			blendString = Utils::RemoveCharFromString(blendString, '\n');
			std::vector<std::string> blends = Utils::SplitString(blendString, ' ');
			GE_CORE_ASSERT(blends.size() == 3 || blends.size() == 1 || blends.size() == 6, "Syntax error");
			modeColor = Utils::ShaderBlendModeFromString(blends.at(0));
			modeAlpha = Utils::ShaderBlendModeFromString(blends.at(0));
			if (blends.size() == 3)
			{
				modeColor = Utils::ShaderBlendModeFromString(blends.at(0));
				modeAlpha = Utils::ShaderBlendModeFromString(blends.at(0));
				srcColor = Utils::ShaderBlendFactorFromString(blends.at(1));
				srcAlpha = Utils::ShaderBlendFactorFromString(blends.at(1));
				dstColor = Utils::ShaderBlendFactorFromString(blends.at(2));
				dstAlpha = Utils::ShaderBlendFactorFromString(blends.at(2));
				GE_CORE_TRACE("Blend type: {0}, Src factor: {1}, Dst factor: {2}", blends.at(0), blends.at(1), blends.at(2));
			}
			else if (blends.size() == 6)
			{
				modeColor = Utils::ShaderBlendModeFromString(blends.at(0));
				srcColor = Utils::ShaderBlendFactorFromString(blends.at(1));
				dstColor = Utils::ShaderBlendFactorFromString(blends.at(2));
				modeAlpha = Utils::ShaderBlendModeFromString(blends.at(3));
				srcAlpha = Utils::ShaderBlendFactorFromString(blends.at(4));
				dstAlpha = Utils::ShaderBlendFactorFromString(blends.at(5));
			}
			else
			{
				GE_CORE_TRACE("Blend type: {0}", blends.at(0));
			}
		}
		static void ProcessShaderDepthWrite(const std::string& source, bool& enableDepthWrite)
		{
			const char* token = "#DepthWrite";
			size_t tokenLength = strlen(token);
			size_t pos = source.find(token, 0);
			if (pos == std::string::npos)
			{
				return;
			}
			size_t eol = source.find_first_of("\r\n", pos);
			GE_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + tokenLength + 1;
			std::string depthMaskProp = source.substr(begin, eol - begin);
			int index = 0;
			while ((index = depthMaskProp.find(' ', index)) != std::string::npos)
			{
				depthMaskProp.erase(index, 1);
			}
			enableDepthWrite = Utils::ShaderBoolFromString(depthMaskProp);
			GE_CORE_TRACE("DepthWrite: {0}", enableDepthWrite);
		}
		static void ProcessShaderDepthTest(const std::string& source, CompareOperation& depthTestOperation)
		{
			const char* token = "#DepthTest";
			size_t tokenLength = strlen(token);
			size_t pos = source.find(token, 0);
			if (pos == std::string::npos)
			{
				return;
			}
			size_t eol = source.find_first_of("\r\n", pos);
			GE_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + tokenLength + 1;
			std::string depthTestProp = source.substr(begin, eol - begin);
			int index = 0;
			while ((index = depthTestProp.find(' ', index)) != std::string::npos)
			{
				depthTestProp.erase(index, 1);
			}
			depthTestOperation = Utils::ShaderCompareOperationFromString(depthTestProp);
			GE_CORE_TRACE("DepthTest: {0}", depthTestProp);
		}
		static void ProcessShaderCull(const std::string& source, CullMode& mode)
		{
			const char* token = "#Cull";
			size_t tokenLength = strlen(token);
			size_t pos = source.find(token, 0);
			if (pos == std::string::npos)
			{
				return;
			}
			size_t eol = source.find_first_of("\r\n", pos);
			GE_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + tokenLength + 1;
			std::string cullProp = source.substr(begin, eol - begin);
			int index = 0;
			while ((index = cullProp.find(' ', index)) != std::string::npos)
			{
				cullProp.erase(index, 1);
			}
			mode = Utils::ShaderCullModeFromString(cullProp);
			GE_CORE_TRACE("Cull: {0}", cullProp);
		}
		static std::string ProcessShaderVertexInput(const std::string& source)
		{
			std::string output;

			const char* token = "#VertexInput";
			const char* endToken = "#EndVertexInput";
			size_t tokenLength = strlen(token);
			size_t pos = source.find(token, 0);
			if (pos == std::string::npos)
			{
				return output;
			}
			size_t eol = source.find_first_of("\r\n", pos);
			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			size_t end = source.find(endToken, pos);
			std::string properties = source.substr(nextLinePos, end - nextLinePos);

			std::vector<std::string> props = Utils::SplitString(properties, '\n');
			for (auto& prop : props)
			{
				//prop = Utils::RemoveCharFromString(prop, ' ');
				prop = Utils::RemoveCharFromString(prop, '\r');
				prop = Utils::RemoveCharFromString(prop, '\n');
				prop = Utils::RemoveCharFromString(prop, '\t');
				prop = Utils::RemoveCharFromString(prop, ';');
			}
			for (auto it = props.begin(); it != props.end();)
			{
				if (it->empty())
					it = props.erase(it);
				else
					++it;
			}
			for (auto prop : props)
			{
				std::string flag = Utils::SplitString(prop, ':').at(1);
				std::string pro = Utils::SplitString(prop, ':').at(0);
				output += "layout(location=" + std::to_string(Utils::ShaderVertexInputFlagToLocation(flag)) + ") in " + pro + ";\n";
			}
			GE_CORE_INFO(output);
			return output;
		}

		static std::string ReadFile(const std::string& path)
		{
			std::string src;
			std::ifstream file(path, std::ios::in | std::ios::binary);

			GE_CORE_ASSERT(file, " file " + path + " not found");

			file.seekg(0, std::ios::end);
			src.resize(file.tellg());
			file.seekg(0, std::ios::beg);
			file.read(&src[0], src.size());
			file.close();
			return src;
		}
	}

	class GENGINE_API Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;

		virtual void SetInt1(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, int* value, uint32_t count) = 0;
		virtual void SetFloat1(const std::string& name, float value) = 0;
		virtual void SetFloat2(const std::string& name, const Vector2& value) = 0;
		virtual void SetFloat3(const std::string& name, const Vector3& value) = 0;
		virtual void SetFloat4(const std::string& name, const Vector4& value) = 0;
		virtual void SetMat4x4(const std::string& name, const Matrix4x4& value) = 0;
		virtual void SetMat4x4Array(const std::string& name, const Matrix4x4* value, const uint32_t count) = 0;

		virtual BlendMode GetBlendModeColor() { return m_BlendModeColor; }
		virtual BlendMode GetBlendModeAlpha() { return m_BlendModeAlpha; }
		virtual CullMode GetCullMode() { return m_CullMode; }
		virtual BlendFactor GetBlendColorSourceFactor() { return m_BlendColorSourceFactor; }
		virtual BlendFactor GetBlendColorDestinationFactor() { return m_BlendColorDestinationFactor; }
		virtual BlendFactor GetBlendAlphaSourceFactor() { return m_BlendAlphaSourceFactor; }
		virtual BlendFactor GetBlendAlphaDestinationFactor() { return m_BlendAlphaDestinationFactor; }
		virtual bool GetEnableDepthWrite()  { return m_EnableDepthWrite; }
		virtual CompareOperation GetDepthTestOperation()  { return m_DepthTestOperation; }

		virtual std::vector<ShaderUniform> GetUniforms()  { return m_UniformCache; };
		virtual const std::string& GetShaderName() const  { return m_Name; }
		virtual void SetShaderName(std::string name)  { m_Name = name; }
		virtual std::vector<ShaderUniformTexture2D> GetTexture2D()  { return m_Texture2DCache; }
		virtual uint32_t GetTexture2DCount()  { return m_Texture2DCache.size(); }
		virtual std::vector<ShaderUniformStorageImage2D> GetStorageImage2D()  { return m_StorageImage2DCache; }
		virtual std::vector<ShaderUniformStorageBuffer> GetStorageBuffer() { return m_StorageBufferCache; }
		virtual std::vector<ShaderUniformCubeMap>	GetCubeMap() { return m_CubeMapCache; }

		virtual std::vector<uint32_t> GetVertexShaderSource() = 0;
		virtual std::vector<uint32_t> GetFragmentShaderSource() = 0;

		static Ref<Shader> Create(const std::string& path);
		static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
	protected:
		
		virtual void SetMacroBool(std::string& source) = 0;
		virtual void SetMacroExp(std::string& source) = 0;
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