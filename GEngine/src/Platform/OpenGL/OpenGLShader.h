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

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual int GetBlendMode() override { return m_BlendType; }
		virtual int GetCullMode() override { return m_CullMode; }
		virtual uint32_t GetBlendSourceFactor() override { return m_BlendSourceFactor; }
		virtual uint32_t GetBlendDestinationFactor() override { return m_BlendDestinationFactor; }

		virtual bool GetEnableDepthWrite() override { return m_EnableDepthWrite; }
		virtual bool GetEnableDepthTest() override { return m_EnableDepthTest; }

		virtual std::vector<ShaderUniform> GetUniforms() override { return m_UniformCache; };
		virtual const std::string& GetShaderName() const override { return m_Name; }
		virtual void SetShaderName(std::string name) override { m_Name = name; }

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
	private:
		std::string ReadFile(const std::string& path);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		void Compile(std::unordered_map<GLenum, std::string>& source);

		void CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources);
		void CompileOrGetOpenGLBinaries(const std::unordered_map<GLenum, std::string>& shaderSources);
		void CreateProgram();
		void Reflect(GLenum stage, const std::vector<uint32_t>& shaderData);
	private:
		uint32_t											m_RendererID;
		std::string											m_FilePath;
		std::string											m_Name;

		std::unordered_map<GLenum, std::vector<uint32_t>>	m_VulkanSPIRV;
		std::unordered_map<GLenum, std::vector<uint32_t>>	m_OpenGLSPIRV;

		std::unordered_map<GLenum, std::string>				m_OpenGLSourceCode;

		std::vector<ShaderUniform>							m_UniformCache;

		int													m_BlendType;
		int													m_CullMode;
		uint32_t											m_BlendSourceFactor;
		uint32_t											m_BlendDestinationFactor;
		bool												m_EnableDepthWrite = true;
		bool												m_EnableDepthTest = true;
	};
}


