#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/Shader.h"
#include <glad/glad.h>


namespace GEngine
{
	class GENGINE_API OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& path);
		OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~OpenGLShader();

		void Bind() const;

		virtual std::vector<uint32_t> GetVertexShaderSource() { return m_OpenGLSPIRV[GL_VERTEX_SHADER]; }
		virtual std::vector<uint32_t> GetFragmentShaderSource() { return m_OpenGLSPIRV[GL_FRAGMENT_SHADER]; }

		virtual void SetInt1(const std::string& name, int value) override;
		virtual void SetIntArray(const std::string& name, int* value, uint32_t count) override;
		virtual void SetFloat1(const std::string& name, float value) override;
		virtual void SetFloat2(const std::string& name, const Vector2& value) override;
		virtual void SetFloat3(const std::string& name, const Vector3& value) override;
		virtual void SetFloat4(const std::string& name, const Vector4& value) override;
		virtual void SetMat4x4(const std::string& name, const Matrix4x4& value) override;
		virtual void SetMat4x4Array(const std::string& name, const Matrix4x4* value, const uint32_t count) override;

		void SetUniformMat4(const std::string& name, const Matrix4x4& matrix);
		void SetUniformInt(const std::string& name, int value);
		void SetUniformFloat(const std::string& name, float value);
		void SetUniformFloat4(const std::string& name, const Vector4& vector);
		void SetUniformTexture2D(const std::string& name, int slot);
	protected:
		virtual void SetMacroBool(std::string& source) override;
		virtual void SetMacroExp(std::string& source) override;
	private:
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);

		void CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources);
		void CompileOrGetOpenGLBinaries(std::unordered_map<GLenum, std::string>& shaderSources);
		void CreateProgram();
		void Reflect(GLenum stage, const std::vector<uint32_t>& shaderData);
	private:
		uint32_t											m_RendererID = 0;

		std::unordered_map<GLenum, std::vector<uint32_t>>	m_VulkanSPIRV;
		std::unordered_map<GLenum, std::vector<uint32_t>>	m_OpenGLSPIRV;

		std::unordered_map<GLenum, std::string>				m_OpenGLSourceCode;

		

		std::vector<std::pair<std::string, bool>>			m_MacroBools = {
			{ ShaderMacroName::GE_ATTACHMENT_UV_STARTS_AT_TOP, false}
		};
		std::vector<std::pair<std::string, std::string>>	m_MacroExps = {
			{ ShaderMacroName::GE_BINDING_START, std::to_string(s_SlotOffset)}
		};
	};
}


