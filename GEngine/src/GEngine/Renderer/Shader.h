#pragma once
#include "GEngine/Core/Core.h"
#include <string>
#include <unordered_map>
#include "GEngine/Math/Math.h"


namespace GEngine
{
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

		virtual const std::string GetShaderName() const = 0;
		virtual void SetShaderName(std::string name) = 0;

		static Ref<Shader> Create(const std::string& path);
		static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
	};

	class GENGINE_API ShaderLibrary
	{
	public:
		void Add(Ref<Shader>& shader);
		void Add(const std::string& name, Ref<Shader>& shader);
		Ref<Shader> Load(const std::string& path);
		Ref<Shader> Get(const std::string& name);
		bool Exists(const std::string& name) { return m_Shaders.find(name) != m_Shaders.end(); }
	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};
}