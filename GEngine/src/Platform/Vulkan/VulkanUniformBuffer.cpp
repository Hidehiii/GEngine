#include "GEpch.h"
#include "VulkanUniformBuffer.h"
#include "VulkanContext.h"
#include "VulkanBuffer.h"

namespace GEngine
{
	std::vector<VkDescriptorSetLayout>		VulkanUniformBuffer::s_DescriptorSetLayouts;

	VulkanUniformBuffer::VulkanUniformBuffer(uint32_t size, uint32_t binding)
	{
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
		if(binding != 0)
			s_DescriptorSetLayouts.push_back(m_DescriptorSetLayout);
		Utils::CreateBuffer(VulkanContext::GetDevice(), size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_UniformBuffer, m_UniformBufferMemory);
		vkMapMemory(VulkanContext::GetDevice(), m_UniformBufferMemory, m_Offset, size, 0, &m_MapData);
	}
	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		if (std::find(s_DescriptorSetLayouts.begin(), s_DescriptorSetLayouts.end(), m_DescriptorSetLayout) != s_DescriptorSetLayouts.end())
		{
			s_DescriptorSetLayouts.erase(std::find(s_DescriptorSetLayouts.begin(), s_DescriptorSetLayouts.end(), m_DescriptorSetLayout));
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
	}
	void VulkanUniformBuffer::RT_SetData(const void* data, uint32_t size, uint32_t offset)
	{
	}
}