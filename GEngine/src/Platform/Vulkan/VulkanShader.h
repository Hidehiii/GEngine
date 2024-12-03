#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/Shader.h"

#include <vulkan/vulkan.h>
namespace GEngine
{
	class GENGINE_API VulkanShader : public Shader
	{
	public:
		VulkanShader(const std::string& path);
		VulkanShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~VulkanShader();

		virtual void Bind() const override;

		virtual std::vector<uint32_t> GetVertexShaderSource() { return m_VulkanSPIRV[ShaderStage::Vertex]; }
		virtual std::vector<uint32_t> GetFragmentShaderSource() { return m_VulkanSPIRV[ShaderStage::Fragment]; }

		virtual void SetInt1(const std::string& name, int value) override;
		virtual void SetIntArray(const std::string& name, int* value, uint32_t count) override;
		virtual void SetFloat1(const std::string& name, float value) override;
		virtual void SetFloat2(const std::string& name, const Vector2& value) override;
		virtual void SetFloat3(const std::string& name, const Vector3& value) override;
		virtual void SetFloat4(const std::string& name, const Vector4& value) override;
		virtual void SetMat4x4(const std::string& name, const Matrix4x4& value) override;
		virtual void SetMat4x4Array(const std::string& name, const Matrix4x4* value, const uint32_t count) override;
	protected:
		virtual void SetMacroBool(std::string& source) override;
		virtual void SetMacroExp(std::string& source) override;
	private:
		std::unordered_map<std::string, std::string> PreProcess(const std::string& source);
		void Compile(std::unordered_map<std::string, std::string>& source);

		void CompileOrGetVulkanBinaries(std::unordered_map<std::string, std::string>& shaderSources);
		void CompileOrGetOpenGLBinaries(const std::unordered_map<std::string, std::string>& shaderSources);
		void Reflect(const std::string stage, const std::vector<uint32_t>& shaderData);
	private:

		std::unordered_map<std::string, std::vector<uint32_t>>	m_VulkanSPIRV;
		std::unordered_map<std::string, std::vector<uint32_t>>	m_OpenGLSPIRV;

		std::unordered_map<std::string, std::string>		m_OpenGLSourceCode;

		std::vector<std::pair<std::string, bool>>			m_MacroBools = {
			{ ShaderMacroName::GE_ATTACHMENT_UV_STARTS_AT_TOP, true}
		};
		std::vector<std::pair<std::string, std::string>>	m_MacroExps = {

		};
	};
}


