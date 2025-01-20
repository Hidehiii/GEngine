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
		virtual ~VulkanShader();

		virtual std::vector<uint32_t> GetVertexShaderSource() { return m_VulkanSPIRV[ShaderStage::Vertex]; }
		virtual std::vector<uint32_t> GetFragmentShaderSource() { return m_VulkanSPIRV[ShaderStage::Fragment]; }

		void CreateShaderModule();
		void DestroyShaderModule();
		VkShaderModule GetShaderModule(std::string stage);
	protected:
		virtual void SetMacroBool(std::string& source) override;
		virtual void SetMacroExp(std::string& source) override;

		VkShaderModule CreateShaderModule(const std::vector<uint32_t>& code);
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
			{ ShaderMacroName::GE_BINDING_START, std::to_string(s_SlotOffset)}
		};

		VkShaderModule m_VertexShaderModule = nullptr, m_FragmentShaderModule = nullptr, m_ComputeShaderModule = nullptr, m_TessellationControlShaderModule = nullptr,
						m_TessellationEvaluationShaderModule = nullptr, m_GeometryShaderModule = nullptr;
	};
}


