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
			vkFreeDescriptorSets(VulkanContext::Get()->GetDevice(), VulkanContext::Get()->GetDescriptorPool(), m_DescriptorSets.size(), m_DescriptorSets.data());
		}
	}
	void VulkanComputePipeline::Compute(uint32_t x, uint32_t y, uint32_t z)
	{
		PrepareCompute();
		ComputeCommand::Compute(x, y, z);
	}
	Ref<Material> VulkanComputePipeline::GetMaterial()
	{
		m_NeedUpdateDescripotrSetFrames = std::pow(2, Renderer::GetFramesInFlight()) - 1;
		return m_Material;
	}
	void VulkanComputePipeline::SetMaterial(Ref<Material>& material)
	{
		m_Material = std::dynamic_pointer_cast<VulkanMaterial>(material);
		m_RecreatePipeline = true;
	}
	void VulkanComputePipeline::CreateDescriptorSetAndLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding>	layoutBindings;
		// 公共ubo
		std::vector<VulkanUniformBuffer*> publicUniformBuffer = VulkanUniformBuffer::GetPublicUniformBuffer();
		for (auto buffer : publicUniformBuffer)
		{
			layoutBindings.push_back(buffer->GetDescriptorSetLayoutBinding());
		}
		// 材质ubo
		layoutBindings.push_back(m_Material->GetUniformBuffer()->GetDescriptorSetLayoutBinding());
		// 贴图绑定
		auto texture2Ds = m_Material->GetTexture2Ds();
		for (auto& texture2D : texture2Ds)
		{
			VkDescriptorSetLayoutBinding		layoutBinding{};
			layoutBinding.binding				= texture2D.Slot;
			layoutBinding.descriptorCount		= 1;
			layoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			layoutBinding.pImmutableSamplers	= nullptr;
			layoutBinding.stageFlags			= VK_SHADER_STAGE_COMPUTE_BIT;

			layoutBindings.push_back(layoutBinding);
		}
		// storage image 绑定
		auto storageImage2Ds = m_Material->GetStorageImage2Ds();
		for (auto& image2D : storageImage2Ds)
		{
			VkDescriptorSetLayoutBinding		layoutBinding{};
			layoutBinding.binding				= image2D.Slot;
			layoutBinding.descriptorCount		= 1;
			layoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			layoutBinding.pImmutableSamplers	= nullptr;
			layoutBinding.stageFlags			= VK_SHADER_STAGE_COMPUTE_BIT;

			layoutBindings.push_back(layoutBinding);
		}
		// storage buffer 绑定
		auto storageBuffers = m_Material->GetStorageBuffers();
		for (auto& buffer : storageBuffers)
		{
			VkDescriptorSetLayoutBinding		layoutBinding{};
			layoutBinding.binding				= buffer.Slot;
			layoutBinding.descriptorCount		= 1;
			layoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			layoutBinding.pImmutableSamplers	= nullptr;
			layoutBinding.stageFlags			= VK_SHADER_STAGE_COMPUTE_BIT;

			layoutBindings.push_back(layoutBinding);
		}
		//cube map 绑定
		auto cubeMaps = m_Material->GetCubeMaps();
		for (auto& cubeMap : cubeMaps)
		{
			VkDescriptorSetLayoutBinding		layoutBinding{};
			layoutBinding.binding				= cubeMap.Slot;
			layoutBinding.descriptorCount		= 1;
			layoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			layoutBinding.pImmutableSamplers	= nullptr;
			layoutBinding.stageFlags			= VK_SHADER_STAGE_COMPUTE_BIT;

			layoutBindings.push_back(layoutBinding);
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount			= static_cast<uint32_t>(layoutBindings.size());
		layoutInfo.pBindings			= layoutBindings.data();

		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(VulkanContext::Get()->GetDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout));

		// 只有一个set，需要兼容gl没有set的概念
		// 创建多个set用于多帧时候的不同绑定和更新，不然会更新到在cmdBuffer里用的set

		m_DescriptorSets.resize(Renderer::GetFramesInFlight());


		VkDescriptorSetAllocateInfo		allocInfo{};
		allocInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool		= VulkanContext::Get()->GetDescriptorPool();
		allocInfo.descriptorSetCount	= 1;
		allocInfo.pSetLayouts			= &m_DescriptorSetLayout;

		for (int i = 0; i < Renderer::GetFramesInFlight(); i++)
		{
			VK_CHECK_RESULT(vkAllocateDescriptorSets(VulkanContext::Get()->GetDevice(), &allocInfo, &m_DescriptorSets[i]));
		}
	}
	void VulkanComputePipeline::UpdateDescriptorSet(int index)
	{
		std::vector<VkWriteDescriptorSet>		writeInfos;
		VkWriteDescriptorSet					descriptorWrite{};
		// 更新公共uniform buffer
		std::vector<VulkanUniformBuffer*> publicUniformBuffer = VulkanUniformBuffer::GetPublicUniformBuffer();
		for (auto buffer : publicUniformBuffer)
		{
			descriptorWrite.sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet			= m_DescriptorSets.at(index);
			descriptorWrite.dstBinding		= buffer->GetBinding();
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo		= buffer->GetDescriptorBufferInfo();
			descriptorWrite.pImageInfo		= nullptr; // Optional
			descriptorWrite.pTexelBufferView = nullptr; // Optional

			writeInfos.push_back(descriptorWrite);
		}
		// 更新材质的uniform buffer
		descriptorWrite.sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet			= m_DescriptorSets.at(index);
		descriptorWrite.dstBinding		= m_Material->GetUniformBuffer()->GetBinding();
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo		= m_Material->GetUniformBuffer()->GetDescriptorBufferInfo();
		descriptorWrite.pImageInfo		= nullptr; // Optional
		descriptorWrite.pTexelBufferView = nullptr; // Optional

		writeInfos.push_back(descriptorWrite);
		// 更新贴图绑定
		auto texture2Ds = m_Material->GetTexture2Ds();
		for (auto& texture2D : texture2Ds)
		{
			descriptorWrite.sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet			= m_DescriptorSets.at(index);
			descriptorWrite.dstBinding		= texture2D.Slot;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType	= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo		= nullptr;
			descriptorWrite.pImageInfo		= std::dynamic_pointer_cast<VulkanTexture2D>(texture2D.Texture)->GetDescriptorImageInfo();
			descriptorWrite.pTexelBufferView = nullptr; // Optional

			writeInfos.push_back(descriptorWrite);
		}
		// 更新cubeMap绑定
		auto cubeMaps = m_Material->GetCubeMaps();
		for (auto& cubeMap : cubeMaps)
		{
			descriptorWrite.sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet			= m_DescriptorSets.at(index);
			descriptorWrite.dstBinding		= cubeMap.Slot;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType	= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo		= nullptr;
			descriptorWrite.pImageInfo		= std::dynamic_pointer_cast<VulkanCubeMap>(cubeMap.Cubemap)->GetDescriptorImageInfo();
			descriptorWrite.pTexelBufferView = nullptr; // Optional

			writeInfos.push_back(descriptorWrite);
		}
		// 更新storage image绑定
		auto storageImage2Ds = m_Material->GetStorageImage2Ds();
		for (auto& image2D : storageImage2Ds)
		{
			descriptorWrite.sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet			= m_DescriptorSets.at(index);
			descriptorWrite.dstBinding		= image2D.Slot;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType	= VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo		= nullptr;
			descriptorWrite.pImageInfo		= std::dynamic_pointer_cast<VulkanStorageImage2D>(image2D.Image)->GetDescriptorImageInfo();
			descriptorWrite.pTexelBufferView = nullptr; // Optional

			writeInfos.push_back(descriptorWrite);
		}
		// 更新storage buffer绑定
		auto storageBuffers = m_Material->GetStorageBuffers();
		for (auto& buffer : storageBuffers)
		{
			descriptorWrite.sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet			= m_DescriptorSets.at(index);
			descriptorWrite.dstBinding		= buffer.Slot;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType	= VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo		= std::dynamic_pointer_cast<VulkanStorageBuffer>(buffer.Buffer)->GetDescriptorBufferInfo();
			descriptorWrite.pImageInfo		= nullptr;
			descriptorWrite.pTexelBufferView = nullptr; // Optional

			writeInfos.push_back(descriptorWrite);
		}

		vkUpdateDescriptorSets(VulkanContext::Get()->GetDevice(), writeInfos.size(), writeInfos.data(), 0, nullptr);
	}
	void VulkanComputePipeline::UpdateAllDescriptorSet()
	{
		for (int i = 0; i < Renderer::GetFramesInFlight(); i++)
		{
			UpdateDescriptorSet(i);
		}
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
		pipelineLayoutInfo.pSetLayouts		= &m_DescriptorSetLayout;

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

		m_Material->UploadData();

		if (m_ComputePipeline == nullptr)
		{
			CreateDescriptorSetAndLayout();
			CreatePipeline();
			UpdateAllDescriptorSet();
		}
		if (m_RecreatePipeline)
		{
			vkDestroyPipeline(VulkanContext::Get()->GetDevice(), m_ComputePipeline, nullptr);
			vkDestroyPipelineLayout(VulkanContext::Get()->GetDevice(), m_PipelineLayout, nullptr);
			vkFreeDescriptorSets(VulkanContext::Get()->GetDevice(), VulkanContext::Get()->GetDescriptorPool(), m_DescriptorSets.size(), m_DescriptorSets.data());
			CreateDescriptorSetAndLayout();
			CreatePipeline();
			UpdateAllDescriptorSet();
		}
		if (m_NeedUpdateDescripotrSetFrames & (uint8_t)(std::pow(2, Renderer::GetCurrentFrame())))
		{
			UpdateDescriptorSet(Renderer::GetCurrentFrame());
			m_NeedUpdateDescripotrSetFrames -= (uint8_t)std::pow(2, Renderer::GetCurrentFrame());
		}


		vkCmdBindPipeline(VulkanContext::Get()->GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipeline);

		vkCmdBindDescriptorSets(VulkanContext::Get()->GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, m_PipelineLayout, 0, 1, &m_DescriptorSets.at(Renderer::GetCurrentFrame()), 0, nullptr);
	}
}