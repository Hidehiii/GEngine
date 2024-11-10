#pragma once
#include "GEngine/Core/Core.h"
#include <string>
#include <unordered_map>
#include "GEngine/Math/Math.h"
#include "GEngine/Core/Buffer.h"
#include "GEngine/Renderer/Texture.h"
#include <algorithm>
#include <filesystem>


namespace GEngine
{
	enum class ShaderUniformType
	{
		None		= 0, 
		Int			= 1,
		Float		= 2,
		Vector		= 3,
		Color		= 4,
		Mat3		= 5,
		Mat4		= 6,
		Sampler2D	= 7,
		SamplerCube = 8
	};
	class GENGINE_API ShaderMacroName
	{
	public:
		static std::string			GE_UV_START_AT_TOP;
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

		std::string Name = "";
		ShaderUniformType Type = ShaderUniformType::None;
		uint32_t Size = 0;
		uint32_t Location = 0;
	};

	struct ShaderUniformTexture2D
	{
		std::string Name;
		uint32_t Slot;
		Ref<Texture2D> Texture;
	};
	class GENGINE_API Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetInt1(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, int* value, uint32_t count) = 0;
		virtual void SetFloat1(const std::string& name, float value) = 0;
		virtual void SetFloat2(const std::string& name, const Vector2& value) = 0;
		virtual void SetFloat3(const std::string& name, const Vector3& value) = 0;
		virtual void SetFloat4(const std::string& name, const Vector4& value) = 0;
		virtual void SetMat4x4(const std::string& name, const Matrix4x4& value) = 0;
		virtual void SetMat4x4Array(const std::string& name, const Matrix4x4* value, const uint32_t count) = 0;

		virtual const std::string& GetShaderName() const = 0;
		virtual void SetShaderName(std::string name) = 0;
		virtual int GetBlendMode() = 0;
		virtual int GetCullMode() = 0;
		virtual uint32_t GetBlendSourceFactor() = 0;
		virtual uint32_t GetBlendDestinationFactor() = 0;
		virtual bool GetEnableDepthWrite() = 0;
		virtual bool GetEnableDepthTest() = 0;
		virtual uint32_t GetTexture2DCount() = 0;

		virtual std::vector<ShaderUniform> GetUniforms() = 0;
		virtual std::vector<ShaderUniformTexture2D> GetTexture2D() = 0;
		virtual std::vector<uint32_t> GetVertexShaderSource() = 0;
		virtual std::vector<uint32_t> GetFragmentShaderSource() = 0;

		static Ref<Shader> Create(const std::string& path);
		static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
	protected:
		virtual void SetMacroBool(std::string& source) = 0;
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

		static bool ShaderBoolFromString(const std::string& value)
		{
			if (ToLower(value) == "on")				return true;
			if (ToLower(value) == "1")				return true;
			if (ToLower(value) == "true")			return true;
			return false;
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
			if (ToLower(type) == "int")			return ShaderUniformType::Int;
			if (ToLower(type) == "float")		return ShaderUniformType::Float;
			if (ToLower(type) == "vector")		return ShaderUniformType::Vector;
			if (ToLower(type) == "color")		return ShaderUniformType::Color;
			if (ToLower(type) == "mat3")		return ShaderUniformType::Mat3;
			if (ToLower(type) == "mat4")		return ShaderUniformType::Mat4;
			if (ToLower(type) == "sampler2d")	return ShaderUniformType::Sampler2D;
			if (ToLower(type) == "samplercube")	return ShaderUniformType::SamplerCube;
			GE_CORE_ASSERT(false, "Unknown shader uniform type! " + type);
			return ShaderUniformType::None;
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
	}
}