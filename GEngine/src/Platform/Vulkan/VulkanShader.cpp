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
		}
	}

	void VulkanShader::ProcessMachineCode(const std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& shaders)
	{
		m_ShaderModules.clear();
		m_ShaderModules.resize(shaders.size());
		for (int i = 0; i < shaders.size(); i++)
		{
			for (auto&& [stage, byte] : shaders[i])
			{
				VkShaderModuleCreateInfo	createInfo{};
				createInfo.sType			= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				createInfo.codeSize			= byte.size();
				createInfo.pCode			= reinterpret_cast<const uint32_t*>(byte.data());
				VkShaderModule				shaderModule;
				VK_CHECK_RESULT(vkCreateShaderModule(VulkanContext::Get()->GetDevice(), &createInfo, nullptr, &shaderModule));

				m_ShaderModules.at(i)[stage] = shaderModule;
			}
		}
	}

}