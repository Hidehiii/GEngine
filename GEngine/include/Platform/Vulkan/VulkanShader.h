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

		const VkDescriptorSetLayout*							GetDescriptorSetLayout(const int& pass) const { return &m_DescriptorSetLayouts.at(pass); }
		const VkPipelineLayout									GetPipelineLayout(const int& pass) const { return m_PipelineLayouts.at(pass); }

		bool operator==(const VulkanShader& other) const
		{
			return m_Name == other.m_Name && m_FilePath == other.m_FilePath;
		}
	private:
		void CreateShaderModules(const std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& shaders);
		void CreateDescriptorSetLayouts();
		void CreatePipelineLayouts();

		virtual void ProcessMachineCode(const std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& shaders) override; // pass { stage : byte}
	private:

		std::vector<std::unordered_map<std::string, VkShaderModule>>	m_ShaderModules; // pass { stage : module} 
		std::vector<VkDescriptorSetLayout>								m_DescriptorSetLayouts;
		std::vector<VkPipelineLayout>									m_PipelineLayouts;
	};
}


