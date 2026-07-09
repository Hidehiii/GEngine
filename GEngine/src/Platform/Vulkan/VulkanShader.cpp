#include "GEpch.h"
#include "VulkanShader.h"
#include "GEngine/Graphics/Material.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanTexture2D.h"


namespace GEngine
{

	VulkanShader::VulkanShader(const std::string& path)
	{
		InitializeShader(path, GE_BIND_CLASS_FUNCTION_LAMBDA(ProcessMachineCode));
		CreateDescriptorSetLayouts();
		CreatePipelineLayouts();
	}
	VulkanShader::~VulkanShader()
	{
		if(VulkanContext::Get()->GetDevice())
		{
			for (auto&& passModules : m_ShaderModules)
			{
				for (auto&& [stage, module] : passModules)
				{
					vkDestroyShaderModule(VulkanContext::Get()->GetDevice(), module, nullptr);
				}
			}
			for (auto& layout : m_PipelineLayouts)
			{
				vkDestroyPipelineLayout(VulkanContext::Get()->GetDevice(), layout, nullptr);
			}
			for(auto& layout : m_DescriptorSetLayouts)
			{
				vkDestroyDescriptorSetLayout(VulkanContext::Get()->GetDevice(), layout, nullptr);
			}
		}
	}

	void VulkanShader::CreateShaderModules(const std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& shaders)
	{
		m_ShaderModules.clear();
		m_ShaderModules.resize(shaders.size());
		for (int i = 0; i < shaders.size(); i++)
		{
			for (auto&& [stage, byte] : shaders[i])
			{
				VkShaderModuleCreateInfo	createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				createInfo.codeSize = byte.size();
				createInfo.pCode = reinterpret_cast<const uint32_t*>(byte.data());
				VkShaderModule				shaderModule;
				VK_CHECK_RESULT(vkCreateShaderModule(VulkanContext::Get()->GetDevice(), &createInfo, nullptr, &shaderModule));

				m_ShaderModules.at(i)[stage] = shaderModule;
			}
		}
	}

	void VulkanShader::CreateDescriptorSetLayouts()
	{
		// we assume all the resources are in set 0, so we only create one set layout for each pass
		m_DescriptorSetLayouts.resize(m_PassReflections.size());
		for (int pass = 0; pass < m_PassReflections.size(); pass++)
		{
			std::vector<VkDescriptorSetLayoutBinding>	layoutBindings;
			VkDescriptorSetLayoutBinding				layoutBinding{};
			const auto& reflection = m_PassReflections.at(pass);
			// create descriptor set layout bindings for each uniform buffer
			for (auto& uboInfo : reflection.CBuffers)
			{
				layoutBinding.binding				= uboInfo.BindPoint;
				layoutBinding.descriptorType		= uboInfo.IsDynamic ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				layoutBinding.descriptorCount		= 1;
				layoutBinding.pImmutableSamplers	= VK_NULL_HANDLE;
				layoutBinding.stageFlags			= VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT;

				layoutBindings.push_back(layoutBinding);
			}
			// create descriptor set layout bindings for each resource
			for (auto& resourceInfo : reflection.Resources)
			{
				layoutBinding.binding				= resourceInfo.BindPoint;
				layoutBinding.descriptorType		= Utils::ShaderPropertyTypeToVkDescriptorType(resourceInfo.Type);
				layoutBinding.descriptorCount		= 1;
				layoutBinding.pImmutableSamplers	= VK_NULL_HANDLE;
				layoutBinding.stageFlags			= VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT;

				layoutBindings.push_back(layoutBinding);
			}

			VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
			layoutCreateInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutCreateInfo.bindingCount	= static_cast<uint32_t>(layoutBindings.size());
			layoutCreateInfo.pBindings		= layoutBindings.data();

			VK_CHECK_RESULT(vkCreateDescriptorSetLayout(VulkanContext::Get()->GetDevice(), &layoutCreateInfo, nullptr, &m_DescriptorSetLayouts[pass]));
		}
	}

	void VulkanShader::CreatePipelineLayouts()
	{
		m_PipelineLayouts.resize(m_PassReflections.size());
		for (int pass = 0; pass < m_PassReflections.size(); pass++)
		{
			VkPipelineLayoutCreateInfo				pipelineLayoutCreateInfo{};
			pipelineLayoutCreateInfo.sType			= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutCreateInfo.setLayoutCount = 1;
			pipelineLayoutCreateInfo.pSetLayouts	= &m_DescriptorSetLayouts[pass];

			VK_CHECK_RESULT(vkCreatePipelineLayout(VulkanContext::Get()->GetDevice(), &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayouts[pass]));
		}
	}

	void VulkanShader::ProcessMachineCode(const std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& shaders)
	{
		CreateShaderModules(shaders);
	}

}