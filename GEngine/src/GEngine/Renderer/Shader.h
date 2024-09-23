#pragma once
#include "GEngine/Core/Core.h"
#include <string>
#include <unordered_map>
#include "GEngine/Math/Math.h"
#include "GEngine/Core/Buffer.h"


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
		Sampler2D	= 7
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
		virtual bool GetEnableDepthMask() = 0;
		virtual bool GetEnableDepthTest() = 0;

		virtual std::vector<ShaderUniform> GetUniforms() = 0;

		static Ref<Shader> Create(const std::string& path);
		static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
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