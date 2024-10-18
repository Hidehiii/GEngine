#include "GEpch.h"
#include "VulkanUniformBuffer.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace GEngine
{
	std::vector<VkDescriptorSetLayout>		VulkanUniformBuffer::s_DescriptorSetLayouts;
	std::vector<VkDescriptorSet>			VulkanUniformBuffer::s_DescriptorSets;

	VulkanUniformBuffer::VulkanUniformBuffer(uint32_t size, uint32_t binding)
	{
		m_Binding						= binding;

		VkDescriptorSetLayoutBinding	uboLayoutBinding{};
		uboLayoutBinding.binding		= binding;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags		= VK_SHADER_STAGE_ALL_GRAPHICS;
		uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount			= 1;
		layoutInfo.pBindings			= &uboLayoutBinding;

		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(VulkanContext::GetDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout));

		VkDescriptorSetAllocateInfo		allocInfo{};
		allocInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool		= VulkanContext::GetDescriptorPool();
		allocInfo.descriptorSetCount	= 1;
		allocInfo.pSetLayouts			= &m_DescriptorSetLayout;

		VK_CHECK_RESULT(vkAllocateDescriptorSets(VulkanContext::GetDevice(), &allocInfo, &m_DescriptorSet));

		if (binding != 0)
		{
			s_DescriptorSetLayouts.push_back(m_DescriptorSetLayout);
			s_DescriptorSets.push_back(m_DescriptorSet);
		}
		Utils::CreateBuffer(VulkanContext::GetPhysicalDevice(), 
							VulkanContext::GetDevice(), 
							size, 
							VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
							m_UniformBuffer, 
							m_UniformBufferMemory);
		vkMapMemory(VulkanContext::GetDevice(), m_UniformBufferMemory, m_Offset, size, 0, &m_MapData);
	}
	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		if (std::find(s_DescriptorSetLayouts.begin(), s_DescriptorSetLayouts.end(), m_DescriptorSetLayout) != s_DescriptorSetLayouts.end())
		{
			s_DescriptorSetLayouts.erase(std::find(s_DescriptorSetLayouts.begin(), s_DescriptorSetLayouts.end(), m_DescriptorSetLayout));
		}
		if (std::find(s_DescriptorSets.begin(), s_DescriptorSets.end(), m_DescriptorSet) != s_DescriptorSets.end())
		{
			s_DescriptorSets.erase(std::find(s_DescriptorSets.begin(), s_DescriptorSets.end(), m_DescriptorSet));
		}
		vkDestroyBuffer(VulkanContext::GetDevice(), m_UniformBuffer, nullptr);
		vkFreeMemory(VulkanContext::GetDevice(), m_UniformBufferMemory, nullptr);
		vkDestroyDescriptorSetLayout(VulkanContext::GetDevice(), m_DescriptorSetLayout, nullptr);
	}
	void VulkanUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		if (offset != m_Offset)
		{
			m_Offset = offset;
			vkUnmapMemory(VulkanContext::GetDevice(), m_UniformBufferMemory);
			vkMapMemory(VulkanContext::GetDevice(), m_UniformBufferMemory, m_Offset, size, 0, &m_MapData);
		}
		memcpy(m_MapData, data, size);

		VkDescriptorBufferInfo			bufferInfo{};
		bufferInfo.buffer				= m_UniformBuffer;
		bufferInfo.offset				= m_Offset;
		bufferInfo.range				= size;

		VkWriteDescriptorSet			descriptorWrite{};
		descriptorWrite.sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet			= m_DescriptorSet;
		descriptorWrite.dstBinding		= m_Offset;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo		= &bufferInfo;
		descriptorWrite.pImageInfo		= nullptr; // Optional
		descriptorWrite.pTexelBufferView = nullptr; // Optional

		vkUpdateDescriptorSets(VulkanContext::GetDevice(), 1, &descriptorWrite, 0, nullptr);
	}
	void VulkanUniformBuffer::RT_SetData(const void* data, uint32_t size, uint32_t offset)
	{
	}
}