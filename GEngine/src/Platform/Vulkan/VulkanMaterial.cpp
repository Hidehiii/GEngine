#include "GEpch.h"
#include "VulkanMaterial.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "GEngine/Renderer/Renderer.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanStorageImage2D.h"
#include "Platform/Vulkan/VulkanCubeMap.h"
#include "Platform/Vulkan/VulkanStorageBuffer.h"

namespace GEngine
{
	VulkanMaterial::VulkanMaterial(const Ref<Shader>& shader, const std::string& name)
	{
		m_Shader = std::dynamic_pointer_cast<VulkanShader>(shader);
		m_Name = name.empty() ? "New Material" : name;
		if (m_Shader)
		{
			uint32_t size = 0;
			for (auto uniform : m_Shader->GetUniforms())
			{
				size += uniform.Size;
				m_Uniforms.push_back(uniform);
			}
			m_UniformsBuffer.Allocate(size);
			m_UniformsBuffer.ZeroInitialize();
			// Create uniform buffer
			// 0 is reserved for custom uniform buffer
			m_UniformBuffer = std::dynamic_pointer_cast<VulkanUniformBuffer>(UniformBuffer::Create(size, 0));
			if (!m_UniformBuffer)
			{
				GE_CORE_CRITICAL("Failed to create uniform buffer for material {0}!", name);
			}
			// Read blend type and factor
			m_BlendModeColor = m_Shader->GetBlendModeColor();
			m_BlendModeAlpha = m_Shader->GetBlendModeAlpha();
			m_BlendColorSourceFactor		= m_Shader->GetBlendColorSourceFactor();
			m_BlendAlphaSourceFactor		= m_Shader->GetBlendAlphaSourceFactor();
			m_BlendColorDestinationFactor	= m_Shader->GetBlendColorDestinationFactor();
			m_BlendAlphaDestinationFactor	= m_Shader->GetBlendAlphaDestinationFactor();
			// cull mode
			m_CullMode					= m_Shader->GetCullMode();
			// Read depth test and depth mask
			m_EnableDepthWrite			= m_Shader->GetEnableDepthWrite();
			m_DepthTestOperation		= m_Shader->GetDepthTestOperation();
			// Texture2D
			m_Texture2D					= m_Shader->GetTexture2D();
			// StorageImage2D
			m_StorageImage2D			= m_Shader->GetStorageImage2D();
			// StorageBuffer
			m_StorageBuffer				= m_Shader->GetStorageBuffer();
			// cube map
			m_CubeMap					= m_Shader->GetCubeMap();

			CreateDescriptorSetAndLayout();
		}
		else
		{
			GE_CORE_WARN("Shader of material {0} is not of type vulaknShader!", name);
		}
	}
	VulkanMaterial::~VulkanMaterial()
	{
		if (VulkanContext::Get()->GetDevice())
		{
			vkFreeDescriptorSets(VulkanContext::Get()->GetDevice(), VulkanContext::Get()->GetDescriptorPool(), m_DescriptorSets.size(), m_DescriptorSets.data());
		}
	}
	void VulkanMaterial::Update()
	{
		if(m_UniformsBuffer.Size > 0)
			m_UniformBuffer->SetData(m_UniformsBuffer.ReadBytes(m_UniformsBuffer.GetSize()), m_UniformsBuffer.GetSize());

		for (auto& texture2D : m_Texture2D)
		{
			texture2D.Texture->Bind(texture2D.Slot);
		}

		for (auto& image2D : m_StorageImage2D)
		{
			image2D.Image->Bind(image2D.Slot);
		}

		for (auto& storageBuffer : m_StorageBuffer)
		{
			storageBuffer.Buffer->Bind(storageBuffer.Slot);
		}

		for (auto& cubeMap : m_CubeMap)
		{
			cubeMap.Cubemap->Bind(cubeMap.Slot);
		}

		if (m_NeedUpdateDescripotrSetFrames & (uint8_t)(std::pow(2, Renderer::GetCurrentFrame())))
		{
			UpdateDescriptorSet(Renderer::GetCurrentFrame());
			m_NeedUpdateDescripotrSetFrames -= (uint8_t)std::pow(2, Renderer::GetCurrentFrame());
		}
	}

	void VulkanMaterial::SetIntArray(const std::string& name, int* value, uint32_t count)
	{
	}

	void VulkanMaterial::SetShader(const Ref<Shader>& shader)
	{
	}
	void VulkanMaterial::SetMatrix4x4(const std::string& name, const Matrix4x4& value)
	{
	}

	void VulkanMaterial::CreateDescriptorSetAndLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding>	layoutBindings;
		// 公共ubo
		std::vector<VulkanUniformBuffer*> publicUniformBuffer = VulkanUniformBuffer::GetPublicUniformBuffer();
		for (auto buffer : publicUniformBuffer)
		{
			layoutBindings.push_back(buffer->GetDescriptorSetLayoutBinding());
		}
		// 材质ubo
		layoutBindings.push_back(m_UniformBuffer->GetDescriptorSetLayoutBinding());
		// 贴图绑定
		for (auto& texture2D : m_Texture2D)
		{
			VkDescriptorSetLayoutBinding		layoutBinding{};
			layoutBinding.binding				= texture2D.Slot;
			layoutBinding.descriptorCount		= 1;
			layoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			layoutBinding.pImmutableSamplers	= nullptr;
			layoutBinding.stageFlags			= VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT;

			layoutBindings.push_back(layoutBinding);
		}
		// storage image 绑定
		for (auto& image2D : m_StorageImage2D)
		{
			VkDescriptorSetLayoutBinding		layoutBinding{};
			layoutBinding.binding				= image2D.Slot;
			layoutBinding.descriptorCount		= 1;
			layoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			layoutBinding.pImmutableSamplers	= nullptr;
			layoutBinding.stageFlags			= VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT;

			layoutBindings.push_back(layoutBinding);
		}
		// storage buffer 绑定
		for (auto& buffer : m_StorageBuffer)
		{
			VkDescriptorSetLayoutBinding		layoutBinding{};
			layoutBinding.binding				= buffer.Slot;
			layoutBinding.descriptorCount		= 1;
			layoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			layoutBinding.pImmutableSamplers	= nullptr;
			layoutBinding.stageFlags			= VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT;

			layoutBindings.push_back(layoutBinding);
		}
		//cube map 绑定
		for (auto& cubeMap : m_CubeMap)
		{
			VkDescriptorSetLayoutBinding		layoutBinding{};
			layoutBinding.binding				= cubeMap.Slot;
			layoutBinding.descriptorCount		= 1;
			layoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			layoutBinding.pImmutableSamplers	= nullptr;
			layoutBinding.stageFlags			= VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT;

			layoutBindings.push_back(layoutBinding);
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount			= static_cast<uint32_t>(layoutBindings.size());
		layoutInfo.pBindings			= layoutBindings.data();

		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(VulkanContext::Get()->GetDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout));

		// 只用一个set，需要兼容gl没有set的概念
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
	void VulkanMaterial::UpdateDescriptorSet(int index)
	{
		std::vector<VkWriteDescriptorSet>		writeInfos;
		VkWriteDescriptorSet					descriptorWrite{};
		// 公共uniform buffer
		std::vector<VulkanUniformBuffer*> publicUniformBuffer = VulkanUniformBuffer::GetPublicUniformBuffer();
		for (auto buffer : publicUniformBuffer)
		{
			descriptorWrite.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet				= m_DescriptorSets.at(index);
			descriptorWrite.dstBinding			= buffer->GetBinding();
			descriptorWrite.dstArrayElement		= 0;
			descriptorWrite.descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount		= 1;
			descriptorWrite.pBufferInfo			= buffer->GetDescriptorBufferInfo();
			descriptorWrite.pImageInfo			= nullptr; // Optional
			descriptorWrite.pTexelBufferView	= nullptr; // Optional

			writeInfos.push_back(descriptorWrite);
		}
		// 材质的uniform buffer
		descriptorWrite.sType					= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet					= m_DescriptorSets.at(index);
		descriptorWrite.dstBinding				= m_UniformBuffer->GetBinding();
		descriptorWrite.dstArrayElement			= 0;
		descriptorWrite.descriptorType			= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount			= 1;
		descriptorWrite.pBufferInfo				= m_UniformBuffer->GetDescriptorBufferInfo();
		descriptorWrite.pImageInfo				= nullptr; // Optional
		descriptorWrite.pTexelBufferView		= nullptr; // Optional

		writeInfos.push_back(descriptorWrite);
		// 贴图绑定
		for (auto& texture2D : m_Texture2D)
		{
			descriptorWrite.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet				= m_DescriptorSets.at(index);
			descriptorWrite.dstBinding			= texture2D.Slot;
			descriptorWrite.dstArrayElement		= 0;
			descriptorWrite.descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount		= 1;
			descriptorWrite.pBufferInfo			= nullptr;
			descriptorWrite.pImageInfo			= std::dynamic_pointer_cast<VulkanTexture2D>(texture2D.Texture)->GetDescriptorImageInfo();
			descriptorWrite.pTexelBufferView	= nullptr; // Optional

			writeInfos.push_back(descriptorWrite);
		}
		// cubeMap绑定;
		for (auto& cubeMap : m_CubeMap)
		{
			descriptorWrite.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet				= m_DescriptorSets.at(index);
			descriptorWrite.dstBinding			= cubeMap.Slot;
			descriptorWrite.dstArrayElement		= 0;
			descriptorWrite.descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount		= 1;
			descriptorWrite.pBufferInfo			= nullptr;
			descriptorWrite.pImageInfo			= std::dynamic_pointer_cast<VulkanCubeMap>(cubeMap.Cubemap)->GetDescriptorImageInfo();
			descriptorWrite.pTexelBufferView	= nullptr; // Optional

			writeInfos.push_back(descriptorWrite);
		}
		// storage image绑定
		for (auto& image2D : m_StorageImage2D)
		{
			descriptorWrite.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet				= m_DescriptorSets.at(index);
			descriptorWrite.dstBinding			= image2D.Slot;
			descriptorWrite.dstArrayElement		= 0;
			descriptorWrite.descriptorType		= VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			descriptorWrite.descriptorCount		= 1;
			descriptorWrite.pBufferInfo			= nullptr;
			descriptorWrite.pImageInfo			= std::dynamic_pointer_cast<VulkanStorageImage2D>(image2D.Image)->GetDescriptorImageInfo();
			descriptorWrite.pTexelBufferView	= nullptr; // Optional

			writeInfos.push_back(descriptorWrite);
		}
		// storage buffer绑定
		for (auto& buffer : m_StorageBuffer)
		{
			descriptorWrite.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet				= m_DescriptorSets.at(index);
			descriptorWrite.dstBinding			= buffer.Slot;
			descriptorWrite.dstArrayElement		= 0;
			descriptorWrite.descriptorType		= VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrite.descriptorCount		= 1;
			descriptorWrite.pBufferInfo			= std::dynamic_pointer_cast<VulkanStorageBuffer>(buffer.Buffer)->GetDescriptorBufferInfo();
			descriptorWrite.pImageInfo			= nullptr;
			descriptorWrite.pTexelBufferView	= nullptr; // Optional

			writeInfos.push_back(descriptorWrite);
		}

		vkUpdateDescriptorSets(VulkanContext::Get()->GetDevice(), writeInfos.size(), writeInfos.data(), 0, nullptr);
	}
	void VulkanMaterial::UpdateAllDescriptorSet()
	{
		for (int i = 0; i < Renderer::GetFramesInFlight(); i++)
		{
			UpdateDescriptorSet(i);
		}
	}
}