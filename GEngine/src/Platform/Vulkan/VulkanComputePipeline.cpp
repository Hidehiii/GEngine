#include "GEpch.h"
#include "VulkanComputePipeline.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "GEngine/Compute/ComputeCommand.h"
#include "Platform/Vulkan/VulkanCubeMap.h"
#include "Platform/Vulkan/VulkanStorageBuffer.h"
#include "Platform/Vulkan/VulkanStorageImage2D.h"
#include "GEngine/Renderer/Renderer.h"

namespace GEngine
{
	VulkanComputePipeline::VulkanComputePipeline(const Ref<Material>& material)
	{
		m_Material = std::dynamic_pointer_cast<VulkanMaterial>(material);

		VkPipelineCacheCreateInfo			pipelineCacheInfo{};
		pipelineCacheInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		pipelineCacheInfo.initialDataSize	= 0;
		pipelineCacheInfo.pInitialData		= nullptr;

		VK_CHECK_RESULT(vkCreatePipelineCache(VulkanContext::Get()->GetDevice(), &pipelineCacheInfo, nullptr, &m_PipelineCache));
	}
	VulkanComputePipeline::~VulkanComputePipeline()
	{
		if (VulkanContext::Get()->GetDevice())
		{
			vkDestroyPipelineCache(VulkanContext::Get()->GetDevice(), m_PipelineCache, nullptr);
			vkDestroyPipeline(VulkanContext::Get()->GetDevice(), m_ComputePipeline, nullptr);
			vkDestroyPipelineLayout(VulkanContext::Get()->GetDevice(), m_PipelineLayout, nullptr);
		}
	}
	void VulkanComputePipeline::Compute(uint32_t x, uint32_t y, uint32_t z)
	{
		PrepareCompute();
		ComputeCommand::Compute(x, y, z);
	}
	Ref<Material> VulkanComputePipeline::GetMaterial()
	{
		return m_Material;
	}
	void VulkanComputePipeline::SetMaterial(Ref<Material>& material)
	{
		m_Material = std::dynamic_pointer_cast<VulkanMaterial>(material);
		m_RecreatePipeline = true;
	}
	void VulkanComputePipeline::CreatePipeline()
	{
		// TODO 
		std::dynamic_pointer_cast<VulkanShader>(m_Material->GetShader())->CreateShaderModule();

		std::string shaderMainFuncName = m_Material->GetShader()->GetShaderMainFuncName().c_str();
		VkPipelineShaderStageCreateInfo		shaderStage;
		if (std::dynamic_pointer_cast<VulkanShader>(m_Material->GetShader())->GetShaderModule(ShaderStage::Compute))
		{
			shaderStage = (Utils::CreatePipelineShaderStage(VK_SHADER_STAGE_COMPUTE_BIT,
				std::dynamic_pointer_cast<VulkanShader>(m_Material->GetShader())->GetShaderModule(ShaderStage::Compute),
				shaderMainFuncName.c_str()));
		}

		VkPipelineLayoutCreateInfo			pipelineLayoutInfo{};
		pipelineLayoutInfo.sType			= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount	= 1;
		pipelineLayoutInfo.pSetLayouts		= m_Material->GetDescriptorSetLayout();

		VK_CHECK_RESULT(vkCreatePipelineLayout(VulkanContext::Get()->GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout));

		VkComputePipelineCreateInfo		pipelineInfo{};
		pipelineInfo.sType				= VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.layout				= m_PipelineLayout;
		pipelineInfo.stage				= shaderStage;

		VK_CHECK_RESULT(vkCreateComputePipelines(VulkanContext::Get()->GetDevice(), m_PipelineCache, 1, &pipelineInfo, nullptr, &m_ComputePipeline));

		

		// TODO 
		std::dynamic_pointer_cast<VulkanShader>(m_Material->GetShader())->DestroyShaderModule();
	}
	void VulkanComputePipeline::PrepareCompute()
	{
		GE_CORE_ASSERT(VulkanContext::Get()->GetCurrentCommandBuffer(), "There is no commandbuffer be using");

		m_Material->Update();

		if (m_ComputePipeline == nullptr)
		{
			CreatePipeline();
		}
		if (m_RecreatePipeline)
		{
			vkDestroyPipeline(VulkanContext::Get()->GetDevice(), m_ComputePipeline, nullptr);
			vkDestroyPipelineLayout(VulkanContext::Get()->GetDevice(), m_PipelineLayout, nullptr);
			CreatePipeline();
		}


		vkCmdBindPipeline(VulkanContext::Get()->GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipeline);

		auto offsets = Renderer::GetDynamicUniformBufferOffsets();
		vkCmdBindDescriptorSets(VulkanContext::Get()->GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, m_PipelineLayout, 0, 1, m_Material->GetDescriptorSet(Renderer::GetCurrentFrame()), offsets.size(), offsets.data());
	}
}