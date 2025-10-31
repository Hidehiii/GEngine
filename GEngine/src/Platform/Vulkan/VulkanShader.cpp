#include "GEpch.h"
#include "VulkanShader.h"
#include "GEngine/Graphics/Material.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanTexture2D.h"


namespace GEngine
{
	namespace Utils
	{
		const char* GLShaderStageCachedVulkanFileExtension(std::string stage)
		{
			if (stage == SHADER_STAGE_VERTEX)					return ".cached_vulkan_vert";
			if (stage == SHADER_STAGE_FRAGMENT)					return ".cached_vulkan_frag";
			if (stage == SHADER_STAGE_COMPUTE)					return ".cached_vulkan_comp";
			if (stage == SHADER_STAGE_HULL)						return ".cached_vulkan_hull";
			if (stage == SHADER_STAGE_DOMAIN)					return ".cached_vulkan_doma";
			if (stage == SHADER_STAGE_GEOMETRY)					return ".cached_vulkan_geom";
			if (stage == SHADER_STAGE_AMPLIFICATION)			return ".cached_vulkan_ampl";
			if (stage == SHADER_STAGE_MESH)						return ".cached_vulkan_mesh";
			GE_CORE_ASSERT(false, "");
			return "";
		}
	}

	VulkanShader::VulkanShader(const std::string& path)
		: Shader(path, GE_BIND_CLASS_FUNCTION_LAMBDA(ProcessMachineCode))
	{
		
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

	void VulkanShader::ProcessMachineCode(const std::vector<std::unordered_map<std::string, std::vector<uint32_t>>>& shaders)
	{
		m_ShaderModules.clear();
		m_ShaderModules.resize(shaders.size());
		for (int i = 0; i < shaders.size(); i++)
		{
			for (auto&& [stage, spirv] : shaders[i])
			{
				VkShaderModuleCreateInfo	createInfo{};
				createInfo.sType			= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				createInfo.codeSize			= spirv.size() * sizeof(uint32_t);
				createInfo.pCode			= spirv.data();
				VkShaderModule				shaderModule;
				VK_CHECK_RESULT(vkCreateShaderModule(VulkanContext::Get()->GetDevice(), &createInfo, nullptr, &shaderModule));

				m_ShaderModules.at(i)[stage] = shaderModule;
			}
		}
	}

}