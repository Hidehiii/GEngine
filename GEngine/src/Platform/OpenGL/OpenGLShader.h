#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/Shader.h"
#include "GEngine/Graphics/GraphicsAPI.h"
#include <glad/glad.h>


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

	protected:
	private:
		std::unordered_map<std::string, std::string> ProcessShaderSource(const std::string& source);
		std::unordered_map<std::string, std::vector<uint32_t>> CompileOpenGLBinaries(std::pair<std::string, ShaderPass> pass);
		uint32_t CreateProgram(std::unordered_map<std::string, std::vector<uint32_t>> shader);
		void Reflect(GLenum stage, const std::vector<uint32_t>& shaderData);
	private:
		std::vector<uint32_t>								m_Programs;


		std::vector<std::pair<std::string, bool>>			m_MacroBools = {
			{ ShaderMacroName::GE_ATTACHMENT_UV_STARTS_AT_TOP, false}
		};
		std::vector<std::pair<std::string, std::string>>	m_MacroExps = {
			{ ShaderMacroName::GE_BINDING_START, std::to_string(s_SlotOffset)},
			{ ShaderMacroName::GE_GRAPHICS_API_OPENGL, std::to_string((int)GraphicsAPI::API::OpenGL)},
			{ ShaderMacroName::GE_GRAPHICS_API_VULKAN, std::to_string((int)GraphicsAPI::API::Vulkan)},
			{ ShaderMacroName::GE_GRAPHICS_API, std::to_string((int)GraphicsAPI::API::OpenGL)},
		};
	};
}


