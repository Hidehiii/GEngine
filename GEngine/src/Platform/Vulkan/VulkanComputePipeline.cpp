#include "GEpch.h"
#include "VulkanComputePipeline.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/Vulkan/VulkanCubeMap.h"
#include "Platform/Vulkan/VulkanStorageBuffer.h"
#include "Platform/Vulkan/VulkanStorageImage2D.h"
#include "VulkanCommandBuffer.h"

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
			for (auto pipeline : m_ComputePipelines)
			{
				vkDestroyPipeline(VulkanContext::Get()->GetDevice(), pipeline.ComputePipeline, nullptr);
			}
			vkDestroyPipelineLayout(VulkanContext::Get()->GetDevice(), m_PipelineLayout, nullptr);
		}
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
	void VulkanComputePipeline::Compute(CommandBuffer* cmdBuffer, const std::string& pass, uint32_t x, uint32_t y, uint32_t z)
	{
		PrepareCompute(cmdBuffer, pass);
		vkCmdDispatch(static_cast<VulkanCommandBuffer*>(cmdBuffer)->GetCommandBuffer(), x, y, z);
	}
	VkPipeline VulkanComputePipeline::GetPipeline(const std::string& pass)
	{
		for (int i = m_ComputePipelines.size() - 1; i >= 0; i--)
		{
			if (m_ComputePipelines.at(i).Pass == pass)
			{
				return m_ComputePipelines.at(i).ComputePipeline;
			}
		}

		std::string shaderMainFuncName = m_Material->GetShader()->GetShaderMainFuncName().c_str();
		VkPipelineShaderStageCreateInfo		shaderStage;
		if (std::dynamic_pointer_cast<VulkanShader>(m_Material->GetShader())->GetShaderModule(ShaderStage::Compute, pass))
		{
			shaderStage = (Utils::CreatePipelineShaderStage(VK_SHADER_STAGE_COMPUTE_BIT,
				std::dynamic_pointer_cast<VulkanShader>(m_Material->GetShader())->GetShaderModule(ShaderStage::Compute, pass),
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

		VkPipeline pipeline;

		VK_CHECK_RESULT(vkCreateComputePipelines(VulkanContext::Get()->GetDevice(), m_PipelineCache, 1, &pipelineInfo, nullptr, &pipeline));

		VulkanComputePipelineInfo	info{};
		info.ComputePipeline		= pipeline;
		info.Pass					= pass;

		m_ComputePipelines.push_back(info);

		return pipeline;
	}
	void VulkanComputePipeline::PrepareCompute(CommandBuffer* cmdBuffer, const std::string& pass)
	{
		m_Material->Update(cmdBuffer, pass);

		if (m_RecreatePipeline)
		{
			for (auto pipeline : m_ComputePipelines)
			{
				vkDestroyPipeline(VulkanContext::Get()->GetDevice(), pipeline.ComputePipeline, nullptr);
			}
			
			vkDestroyPipelineLayout(VulkanContext::Get()->GetDevice(), m_PipelineLayout, nullptr);
		}


		vkCmdBindPipeline(static_cast<VulkanCommandBuffer*>(cmdBuffer)->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, GetPipeline(pass));

		auto offsets = UniformBufferDynamic::GetGlobalUniformOffsets();
		vkCmdBindDescriptorSets(static_cast<VulkanCommandBuffer*>(cmdBuffer)->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, m_PipelineLayout, 0, 1, m_Material->GetDescriptorSet(Graphics::GetFrame()), offsets.size(), offsets.data());
	}
}