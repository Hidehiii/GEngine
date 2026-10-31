#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/Shader.h"
#include "GEngine/Graphics/GraphicsAPI.h"
#include <glad/glad.h>
#include <optional>


namespace GEngine
{
	class GENGINE_API OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& path);
		virtual ~OpenGLShader();

		void Use(const int& pass);

		virtual void SetInt1(const std::string& name, int value, const int& pass);
		virtual void SetIntArray(const std::string& name, int* value, uint32_t count, const int& pass);
		virtual void SetFloat1(const std::string& name, float value, const int& pass);
		virtual void SetFloat2(const std::string& name, const Vector2& value, const int& pass);
		virtual void SetFloat3(const std::string& name, const Vector3& value, const int& pass);
		virtual void SetFloat4(const std::string& name, const Vector4& value, const int& pass);
		virtual void SetMat4x4(const std::string& name, const Matrix4x4& value, const int& pass);
		virtual void SetMat4x4Array(const std::string& name, const Matrix4x4* value, const uint32_t count, const int& pass);

		std::string GetCombinedSamplerName(const std::string& textureName) const;
		std::optional<uint32_t> GetCombinedSamplerUnit(const std::string& samplerName) const;

		bool operator==(const OpenGLShader& other) const
		{
			return m_Name == other.m_Name && m_FilePath == other.m_FilePath;
		}
	private:
		virtual void ProcessMachineCode(const std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& shaders) override; // pass { stage : byte}
	private:
		std::vector<uint32_t>								m_Programs;
		std::unordered_map<std::string, std::string>		m_CombinedSamplerNames;
		std::unordered_map<std::string, uint32_t>			m_CombinedSamplerUnits;
	};
}


