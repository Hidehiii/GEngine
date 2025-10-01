#include "GEpch.h"
#include "VulkanMaterial.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "GEngine/Graphics/Graphics.h"
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
		GE_CORE_ASSERT(m_Shader, "Shader is null!");
		uint32_t size = InitializePropertiesMemory();
		// Create uniform buffer
		// 0 is reserved for custom uniform buffer
		if (size > 0)
		{
			m_UniformBuffer = std::dynamic_pointer_cast<VulkanUniformBuffer>(UniformBuffer::Create(size, 0));
		}
		
		m_Passes = std::vector<ShaderPass>(m_Shader->GetPasses());

		CreateDescriptorSetAndLayout();
	}
	VulkanMaterial::~VulkanMaterial()
	{
		if (VulkanContext::Get()->GetDevice())
		{
			vkFreeDescriptorSets(VulkanContext::Get()->GetDevice(), VulkanContext::Get()->GetDescriptorPool(), m_DescriptorSets.size(), m_DescriptorSets.data());
		}
	}

	void VulkanMaterial::Update(CommandBuffer* cmdBuffer, const int& pass)
	{
		if (m_UniformBuffer)
			m_UniformBuffer->SetData(m_Passes.at(pass).ConstProperties.ReadBytes(), m_Passes.at(pass).ConstProperties.GetSize());

		for (auto&& [name, prop] : m_Passes.at(pass).ReferenceProperties)
		{
			auto propertyTypes = GetShader()->GetProperties();
			GE_CORE_ASSERT(propertyTypes.find(name) != propertyTypes.end(), "Could not find type of property {}!", name);
			switch (propertyTypes[name])
			{
			case ShaderPropertyType::Sampler2D:
			{
				(*((Ref<Texture2D>*)prop.Ptr))->Bind(cmdBuffer, prop.Location);
				break;
			}
			case ShaderPropertyType::SamplerCube:
			{
				(*((Ref<CubeMap>*)prop.Ptr))->Bind(cmdBuffer, prop.Location);
				break;
			}
			case ShaderPropertyType::Sampler2DArray:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case ShaderPropertyType::Sampler:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case ShaderPropertyType::Texture2D:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case ShaderPropertyType::TextureCube:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case ShaderPropertyType::Texture2DArray:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case ShaderPropertyType::StorageImage2D:
			{
				(*((Ref<StorageImage2D>*)prop.Ptr))->Bind(cmdBuffer, prop.Location);
				break;
			}
			case ShaderPropertyType::StorageBuffer:
			{
				(*((Ref<StorageBuffer>*)prop.Ptr))->Bind(prop.Location);
				break;
			}
			default:
				break;
			}
		}

		// 按需更新？
		if (m_NeedUpdateDescripotrSetFrames & (uint8_t)(std::pow(2, Graphics::GetFrame())))
		{
			UpdateDescriptorSet(pass, Graphics::GetFrame());
			m_NeedUpdateDescripotrSetFrames -= (uint8_t)std::pow(2, Graphics::GetFrame());
		}


		//UpdateDescriptorSet(pass, Graphics::GetFrame());
	}

	void VulkanMaterial::SetShader(const Ref<Shader>& shader)
	{
		GE_CORE_CRITICAL("暂时还没有写材质的Shader更换");
	}

	void VulkanMaterial::CreateDescriptorSetAndLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding>	layoutBindings;
		// 公共ubo
		std::vector<Ref<UniformBufferDynamic>> globalUniformBuffer = UniformBufferDynamic::GetGlobalUniforms();
		for (auto buffer : globalUniformBuffer)
		{
			layoutBindings.push_back(std::dynamic_pointer_cast<VulkanUniformBufferDynamic>(buffer)->GetDescriptorSetLayoutBinding());
		}
		// 材质ubo
		if (m_UniformBuffer)
		{
			layoutBindings.push_back(m_UniformBuffer->GetDescriptorSetLayoutBinding());
		}

		m_DescriptorSetLayouts.resize(m_Passes.size());
		m_DescriptorSets.resize(m_Passes.size() * Graphics::GetFrameCount());
		for (int pass = 0; pass < m_Passes.size(); pass++)
		{
			for (auto&& [name, prop] : m_Passes.at(pass).ReferenceProperties)
			{
				auto propertyTypes = GetShader()->GetProperties();
				GE_CORE_ASSERT(propertyTypes.find(name) != propertyTypes.end(), "Could not find type of property {}!", name);
				VkDescriptorSetLayoutBinding		layoutBinding{};
				layoutBinding.binding				= prop.Location;
				layoutBinding.descriptorCount		= 1;
				layoutBinding.pImmutableSamplers	= nullptr;
				layoutBinding.stageFlags			= VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT;
				switch (propertyTypes[name])
				{
				case ShaderPropertyType::Sampler2D:
				{
					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					
					layoutBindings.push_back(layoutBinding);
					break;
				}
				case ShaderPropertyType::SamplerCube:
				{
					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

					layoutBindings.push_back(layoutBinding);
					break;
				}
				case ShaderPropertyType::Sampler2DArray:
				{
					GE_CORE_ASSERT(false, "");
					break;
				}
				case ShaderPropertyType::Sampler:
				{
					GE_CORE_ASSERT(false, "");
					break;
				}
				case ShaderPropertyType::Texture2D:
				{
					GE_CORE_ASSERT(false, "");
					break;
				}
				case ShaderPropertyType::TextureCube:
				{
					GE_CORE_ASSERT(false, "");
					break;
				}
				case ShaderPropertyType::Texture2DArray:
				{
					GE_CORE_ASSERT(false, "");
					break;
				}
				case ShaderPropertyType::StorageImage2D:
				{
					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

					layoutBindings.push_back(layoutBinding);
				}
				case ShaderPropertyType::StorageBuffer:
				{
					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

					layoutBindings.push_back(layoutBinding);
				}
				default:
					break;
				}
			}

			// 每个pass都有自己的layout
			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount			= static_cast<uint32_t>(layoutBindings.size());
			layoutInfo.pBindings			= layoutBindings.data();

			VK_CHECK_RESULT(vkCreateDescriptorSetLayout(VulkanContext::Get()->GetDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayouts[pass]));

			// 只用一个set，需要兼容gl没有set的概念
			// 创建多个set用于多帧时候的不同绑定和更新，不然会更新到在cmdBuffer里用的set
			VkDescriptorSetAllocateInfo		allocInfo{};
			allocInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool		= VulkanContext::Get()->GetDescriptorPool();
			allocInfo.descriptorSetCount	= 1;
			allocInfo.pSetLayouts			= &m_DescriptorSetLayouts.at(pass);

			for (int i = 0; i < Graphics::GetFrameCount(); i++)
			{
				VK_CHECK_RESULT(vkAllocateDescriptorSets(VulkanContext::Get()->GetDevice(), &allocInfo, &m_DescriptorSets[pass * Graphics::GetFrameCount() + i]));
			}
		}

		
	}
	void VulkanMaterial::UpdateDescriptorSet(const int& pass, const int& index)
	{
		std::vector<VkWriteDescriptorSet>		writeInfos;
		VkWriteDescriptorSet					descriptorWrite{};
		// 公共uniform buffer
		std::vector<Ref<UniformBufferDynamic>> globalUniformBuffer = UniformBufferDynamic::GetGlobalUniforms();
		for (auto buffer : globalUniformBuffer)
		{
			Ref<VulkanUniformBufferDynamic> buf		= std::dynamic_pointer_cast<VulkanUniformBufferDynamic>(buffer);

			descriptorWrite.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet				= m_DescriptorSets.at(pass * Graphics::GetFrameCount() + index);
			descriptorWrite.dstBinding			= buf->GetDescriptorSetLayoutBinding().binding;
			descriptorWrite.dstArrayElement		= 0;
			descriptorWrite.descriptorType		= buf->GetDescriptorSetLayoutBinding().descriptorType;
			descriptorWrite.descriptorCount		= 1;
			descriptorWrite.pBufferInfo			= buf->GetDescriptorBufferInfo();
			descriptorWrite.pImageInfo			= nullptr; // Optional
			descriptorWrite.pTexelBufferView	= nullptr; // Optional

			writeInfos.push_back(descriptorWrite);
		}
		// 材质的uniform buffer
		descriptorWrite.sType					= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet					= m_DescriptorSets.at(pass * Graphics::GetFrameCount() + index);
		descriptorWrite.dstBinding				= m_UniformBuffer->GetDescriptorSetLayoutBinding().binding;
		descriptorWrite.dstArrayElement			= 0;
		descriptorWrite.descriptorType			= m_UniformBuffer->GetDescriptorSetLayoutBinding().descriptorType;
		descriptorWrite.descriptorCount			= 1;
		descriptorWrite.pBufferInfo				= m_UniformBuffer->GetDescriptorBufferInfo();
		descriptorWrite.pImageInfo				= nullptr; // Optional
		descriptorWrite.pTexelBufferView		= nullptr; // Optional

		writeInfos.push_back(descriptorWrite);

		for (auto&& [name, prop] : m_Passes.at(pass).ReferenceProperties)
		{
			auto propertyTypes = GetShader()->GetProperties();
			GE_CORE_ASSERT(propertyTypes.find(name) != propertyTypes.end(), "Could not find type of property {}!", name);

			descriptorWrite.sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet			= m_DescriptorSets.at(index);
			descriptorWrite.dstBinding		= prop.Location;
			descriptorWrite.descriptorCount = 1;

			switch (propertyTypes[name])
			{
			case ShaderPropertyType::Sampler2D:
			{
				descriptorWrite.dstArrayElement		= 0;
				descriptorWrite.descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.pBufferInfo			= nullptr;
				descriptorWrite.pImageInfo			= std::dynamic_pointer_cast<VulkanTexture2D>((*((Ref<Texture2D>*)prop.Ptr)))->GetDescriptorImageInfo();
				descriptorWrite.pTexelBufferView	= nullptr; // Optional

				writeInfos.push_back(descriptorWrite);
				break;
			}
			case ShaderPropertyType::SamplerCube:
			{
				descriptorWrite.dstArrayElement		= 0;
				descriptorWrite.descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.pBufferInfo			= nullptr;
				descriptorWrite.pImageInfo			= std::dynamic_pointer_cast<VulkanCubeMap>((*((Ref<CubeMap>*)prop.Ptr)))->GetDescriptorImageInfo();
				descriptorWrite.pTexelBufferView	= nullptr; // Optional

				writeInfos.push_back(descriptorWrite);
				break;
			}
			case ShaderPropertyType::Sampler2DArray:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case ShaderPropertyType::Sampler:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case ShaderPropertyType::Texture2D:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case ShaderPropertyType::TextureCube:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case ShaderPropertyType::Texture2DArray:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case ShaderPropertyType::StorageImage2D:
			{
				descriptorWrite.dstArrayElement		= 0;
				descriptorWrite.descriptorType		= VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				descriptorWrite.pBufferInfo			= nullptr;
				descriptorWrite.pImageInfo			= std::dynamic_pointer_cast<VulkanStorageImage2D>((*((Ref<StorageImage2D>*)prop.Ptr)))->GetDescriptorImageInfo();
				descriptorWrite.pTexelBufferView	= nullptr; // Optional

				writeInfos.push_back(descriptorWrite);
				break;
			}
			case ShaderPropertyType::StorageBuffer:
			{
				descriptorWrite.dstArrayElement		= 0;
				descriptorWrite.descriptorType		= VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorWrite.descriptorCount		= 1;
				descriptorWrite.pBufferInfo			= std::dynamic_pointer_cast<VulkanStorageBuffer>((*((Ref<StorageBuffer>*)prop.Ptr)))->GetDescriptorBufferInfo();
				descriptorWrite.pImageInfo			= nullptr;
				descriptorWrite.pTexelBufferView	= nullptr; // Optional

				writeInfos.push_back(descriptorWrite);
				break;
			}
			default:
				break;
			}
		}

		vkUpdateDescriptorSets(VulkanContext::Get()->GetDevice(), writeInfos.size(), writeInfos.data(), 0, nullptr);
	}
}