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
		std::vector<std::unordered_map<uint32_t, uint32_t>> sizes = InitializePassPropertiesMemory();
		// Create uniform buffer
		for(auto& pass : sizes)
		{
			std::unordered_map<uint32_t, Ref<VulkanUniformBuffer>> ubuffers;
			for (auto& [bindPoint, size] : pass)
			{
				if (size > 0)
				{
					Ref<VulkanUniformBuffer> ubo = CreateRef<VulkanUniformBuffer>(size);
					ubuffers[bindPoint] = ubo;
				}
			}
			m_UniformBuffers.push_back(ubuffers);
		}

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
		for (auto& [bindPoint, ubo] : m_UniformBuffers.at(pass))
		{
			GE_CORE_ASSERT(ubo, "Uniform buffer is null!");
			GE_CORE_ASSERT(m_Passes.at(pass).CBuffers.find(bindPoint) != m_Passes.at(pass).CBuffers.end(), "Uniform buffer bind point not found in pass!");
			if (ubo->IsDynamic() == false)
			{
				GE_CORE_ASSERT(m_Passes.at(pass).CBuffers.at(bindPoint).Data, "CBuffer is NULL!");
				ubo->SetData(m_Passes.at(pass).CBuffers.at(bindPoint).ReadBytes(), m_Passes.at(pass).CBuffers.at(bindPoint).GetSize());
			}
		}

		for (auto&& [name, prop] : m_Passes.at(pass).ResourceProperties)
		{
			auto type = m_Shader->GetPropertyType(name);
			switch (type)
			{
			case SHADER_PROPERTY_TYPE_SAMPLER_2D:
			{
				(*((Ref<Texture2DCombineSampler>*)prop.Ptr))->Bind(cmdBuffer, prop.Location);
				break;
			}
			case SHADER_PROPERTY_TYPE_SAMPLER_CUBE:
			{
				(*((Ref<CubeMap>*)prop.Ptr))->Bind(cmdBuffer, prop.Location);
				break;
			}
			case SHADER_PROPERTY_TYPE_SAMPLER_2D_ARRAY:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case SHADER_PROPERTY_TYPE_SAMPLER:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case SHADER_PROPERTY_TYPE_TEXTURE_2D:
			{
				(*((Ref<Texture2D>*)prop.Ptr))->Bind(cmdBuffer, prop.Location);
				break;
			}
			case SHADER_PROPERTY_TYPE_TEXTURE_CUBE:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case SHADER_PROPERTY_TYPE_TEXTURE_2D_ARRAY:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case SHADER_PROPERTY_TYPE_STORAGE_IMAGE_2D:
			{
				(*((Ref<StorageImage2D>*)prop.Ptr))->Bind(cmdBuffer, prop.Location);
				break;
			}
			case SHADER_PROPERTY_TYPE_STORAGE_BUFFER:
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

	Buffer VulkanMaterial::SetUniformBuffer(const int& pass, const uint32_t& bindPoint, const Buffer& buffer, const Ref<UniformBuffer>& buf)
	{
		GE_CORE_ASSERT(m_UniformBuffers.size() > pass, "Pass index out of range!");
		auto& ubuffers = m_UniformBuffers.at(pass);
		GE_CORE_ASSERT(ubuffers.find(bindPoint) != ubuffers.end(), "Uniform buffer bind point not found!");
		ubuffers.at(bindPoint) = std::dynamic_pointer_cast<VulkanUniformBuffer>(buf);
		Buffer oldBuffer = m_Passes.at(pass).CBuffers.at(bindPoint);
		m_Passes.at(pass).CBuffers[bindPoint] = buffer;
		if (buf->IsDynamic())
		{
			UpdateDynamicOffsets(pass);
		}
		return oldBuffer;
	}

	void VulkanMaterial::CreateDescriptorSetAndLayout()
	{
		

		m_DescriptorSetLayouts.resize(m_Passes.size());
		m_DescriptorSets.resize(m_Passes.size() * Graphics::GetFrameCount());
		for (int pass = 0; pass < m_Passes.size(); pass++)
		{
			std::vector<VkDescriptorSetLayoutBinding>	layoutBindings;

			// 材质ubo
			for(auto& [bindPoint, ubo] : m_UniformBuffers.at(pass))
			{
				VkDescriptorSetLayoutBinding		layoutBinding{};
				layoutBinding.binding				= bindPoint;
				layoutBinding.descriptorType		= ubo->GetDescriptorType();
				layoutBinding.descriptorCount		= 1;
				layoutBinding.pImmutableSamplers	= nullptr;
				layoutBinding.stageFlags			= VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT;

				layoutBindings.push_back(layoutBinding);
			}

			for (auto&& [name, prop] : m_Passes.at(pass).ResourceProperties)
			{
				auto type = m_Shader->GetPropertyType(name);
				VkDescriptorSetLayoutBinding		layoutBinding{};
				layoutBinding.binding				= prop.Location;
				layoutBinding.descriptorCount		= 1;
				layoutBinding.pImmutableSamplers	= nullptr;
				layoutBinding.stageFlags			= VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT;
				switch (type)
				{
				case SHADER_PROPERTY_TYPE_SAMPLER_2D:
				{
					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					
					layoutBindings.push_back(layoutBinding);
					break;
				}
				case SHADER_PROPERTY_TYPE_SAMPLER_CUBE:
				{
					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

					layoutBindings.push_back(layoutBinding);
					break;
				}
				case SHADER_PROPERTY_TYPE_SAMPLER_2D_ARRAY:
				{
					GE_CORE_ASSERT(false, "");
					break;
				}
				case SHADER_PROPERTY_TYPE_SAMPLER:
				{
					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

					layoutBindings.push_back(layoutBinding);
					break;
				}
				case SHADER_PROPERTY_TYPE_TEXTURE_2D:
				{
					GE_CORE_ASSERT(false, "");
					break;
				}
				case SHADER_PROPERTY_TYPE_TEXTURE_CUBE:
				{
					GE_CORE_ASSERT(false, "");
					break;
				}
				case SHADER_PROPERTY_TYPE_TEXTURE_2D_ARRAY:
				{
					GE_CORE_ASSERT(false, "");
					break;
				}
				case SHADER_PROPERTY_TYPE_STORAGE_IMAGE_2D:
				{
					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

					layoutBindings.push_back(layoutBinding);
					break;
				}
				case SHADER_PROPERTY_TYPE_STORAGE_BUFFER:
				{
					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

					layoutBindings.push_back(layoutBinding);
					break;
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
		for(auto& [bindPoint, ubo] : m_UniformBuffers.at(pass))
		{
			descriptorWrite.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet				= m_DescriptorSets.at(pass * Graphics::GetFrameCount() + index);
			descriptorWrite.dstBinding			= bindPoint;
			descriptorWrite.dstArrayElement		= 0;
			descriptorWrite.descriptorType		= ubo->GetDescriptorType();
			descriptorWrite.descriptorCount		= 1;
			descriptorWrite.pBufferInfo			= ubo->GetDescriptorBufferInfo();
			descriptorWrite.pImageInfo			= nullptr; // Optional
			descriptorWrite.pTexelBufferView	= nullptr; // Optional
			writeInfos.push_back(descriptorWrite);
		}

		for (auto&& [name, prop] : m_Passes.at(pass).ResourceProperties)
		{
			auto type = m_Shader->GetPropertyType(name);

			descriptorWrite.sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet			= m_DescriptorSets.at(index);
			descriptorWrite.dstBinding		= prop.Location;
			descriptorWrite.descriptorCount = 1;

			switch (type)
			{
			case SHADER_PROPERTY_TYPE_SAMPLER_2D:
			{
				descriptorWrite.dstArrayElement		= 0;
				descriptorWrite.descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.pBufferInfo			= nullptr;
				descriptorWrite.pImageInfo			= std::dynamic_pointer_cast<VulkanTexture2D>((*((Ref<Texture2D>*)prop.Ptr)))->GetDescriptorImageInfo();
				descriptorWrite.pTexelBufferView	= nullptr; // Optional

				writeInfos.push_back(descriptorWrite);
				break;
			}
			case SHADER_PROPERTY_TYPE_SAMPLER_CUBE:
			{
				descriptorWrite.dstArrayElement		= 0;
				descriptorWrite.descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.pBufferInfo			= nullptr;
				descriptorWrite.pImageInfo			= std::dynamic_pointer_cast<VulkanCubeMap>((*((Ref<CubeMap>*)prop.Ptr)))->GetDescriptorImageInfo();
				descriptorWrite.pTexelBufferView	= nullptr; // Optional

				writeInfos.push_back(descriptorWrite);
				break;
			}
			case SHADER_PROPERTY_TYPE_SAMPLER_2D_ARRAY:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case SHADER_PROPERTY_TYPE_SAMPLER:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case SHADER_PROPERTY_TYPE_TEXTURE_2D:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case SHADER_PROPERTY_TYPE_TEXTURE_CUBE:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case SHADER_PROPERTY_TYPE_TEXTURE_2D_ARRAY:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case SHADER_PROPERTY_TYPE_STORAGE_IMAGE_2D:
			{
				descriptorWrite.dstArrayElement		= 0;
				descriptorWrite.descriptorType		= VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				descriptorWrite.pBufferInfo			= nullptr;
				descriptorWrite.pImageInfo			= std::dynamic_pointer_cast<VulkanStorageImage2D>((*((Ref<StorageImage2D>*)prop.Ptr)))->GetDescriptorImageInfo();
				descriptorWrite.pTexelBufferView	= nullptr; // Optional

				writeInfos.push_back(descriptorWrite);
				break;
			}
			case SHADER_PROPERTY_TYPE_STORAGE_BUFFER:
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
	void VulkanMaterial::UpdateDynamicOffsets(const int& pass)
	{
		m_DynamicUniformBufferOffsets.at(pass).clear();
		// sort, from low to high
		std::map<uint32_t, uint32_t> ofts;
		for (auto& [bindPoint, ubo] : m_UniformBuffers.at(pass))
		{
			if (ubo->IsDynamic())
			{
				ofts[bindPoint] = ubo->GetOffset();
			}
		}
		for (auto& [bindPoint, oft] : ofts)
		{
			m_DynamicUniformBufferOffsets.at(pass).push_back(oft);
		}
	}
}