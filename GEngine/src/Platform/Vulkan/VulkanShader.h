#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/Shader.h"
#include "GEngine/Graphics/GraphicsAPI.h"
#include <vulkan/vulkan.h>
namespace GEngine
{

	class GENGINE_API VulkanShader : public Shader
	{
	public:
		VulkanShader(const std::string& path);
		virtual ~VulkanShader();

		const VkShaderModule&									GetShaderModule(const std::string& stage, const int& pass) { return m_ShaderModules.at(pass)[stage]; }
		const std::unordered_map<std::string, VkShaderModule>&	GetShaderModules(const int& pass) { return m_ShaderModules.at(pass); }
	protected:
		virtual bool Compile(std::vector<std::vector<std::string>>& passStages, std::vector<std::string>& shaderSrcCodes) override;

		VkShaderModule CreateShaderModule(const std::vector<uint32_t>& code);
	private:
		std::unordered_map<std::string, std::vector<uint32_t>> CompileVulkanBinaries(std::pair<std::string, ShaderPass> pass);
		void Reflect(const std::string stage, const std::vector<uint32_t>& shaderData);
	private:

		std::vector<std::pair<std::string, bool>>			m_MacroBools = {
			{ ShaderMacroName::GE_ATTACHMENT_UV_STARTS_AT_TOP, true}
		};
		std::vector<std::pair<std::string, std::string>>	m_MacroExps = {
			{ ShaderMacroName::GE_BINDING_START, std::to_string(s_SlotOffset)},
			{ ShaderMacroName::GE_GRAPHICS_API_OPENGL, std::to_string((int)GraphicsAPI::API::OpenGL)},
			{ ShaderMacroName::GE_GRAPHICS_API_VULKAN, std::to_string((int)GraphicsAPI::API::Vulkan)},
			{ ShaderMacroName::GE_GRAPHICS_API, std::to_string((int)GraphicsAPI::API::Vulkan)},
		};

		std::vector<std::unordered_map<std::string, VkShaderModule>>	m_ShaderModules; // stage : module 
	};
}


