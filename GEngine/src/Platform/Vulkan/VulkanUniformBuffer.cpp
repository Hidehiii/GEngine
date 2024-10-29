#include "GEpch.h"
#include "VulkanUniformBuffer.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanContext.h"


bool operator == (VkDescriptorSetLayoutBinding& a, const VkDescriptorSetLayoutBinding& b)
{
	return a.binding == b.binding
		&& a.descriptorType == b.descriptorType
		&& a.descriptorCount == b.descriptorCount
		&& a.stageFlags == b.stageFlags
		&& a.pImmutableSamplers == b.pImmutableSamplers;
}

namespace GEngine
{
	std::vector<VkDescriptorSetLayoutBinding>	VulkanUniformBuffer::s_DescriptorSetLayoutBindings;
	VkDescriptorSetLayout						VulkanUniformBuffer::s_DescriptorSetLayout;
	VkDescriptorSet								VulkanUniformBuffer::s_DescriptorSet;

	VulkanUniformBuffer::VulkanUniformBuffer(uint32_t size, uint32_t binding)
	{
		m_Binding						= binding;
		size = size <= 0 ? 1 : size;

		m_DescriptorSetLayoutBinding.binding		= binding;
		m_DescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		m_DescriptorSetLayoutBinding.descriptorCount = 1;
		m_DescriptorSetLayoutBinding.stageFlags		= VK_SHADER_STAGE_ALL_GRAPHICS;
		m_DescriptorSetLayoutBinding.pImmutableSamplers = nullptr; // Optional
		
		Utils::CreateBuffer(VulkanContext::GetPhysicalDevice(), 
							VulkanContext::GetDevice(), 
							size, 
							VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
							m_UniformBuffer, 
							m_UniformBufferMemory);
		vkMapMemory(VulkanContext::GetDevice(), m_UniformBufferMemory, m_Offset, size, 0, &m_MapData);


		// 公共ubo不会每次更新
		if (binding != 0)
		{
			AddDescriptorSetLayoutBinding(m_DescriptorSetLayoutBinding);
		}
	}
	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		vkDestroyBuffer(VulkanContext::GetDevice(), m_UniformBuffer, nullptr);
		vkFreeMemory(VulkanContext::GetDevice(), m_UniformBufferMemory, nullptr);
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
		descriptorWrite.dstSet			= s_DescriptorSet;
		descriptorWrite.dstBinding		= m_Binding;
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
	void VulkanUniformBuffer::AddDescriptorSetLayoutBinding(VkDescriptorSetLayoutBinding layoutBinding)
	{
		s_DescriptorSetLayoutBindings.push_back(layoutBinding);
		CreateDescriptorSetLayout();
		CreateDescriptorSet();
	}
	void VulkanUniformBuffer::RemoveDescriptorSetLayoutBinding(VkDescriptorSetLayoutBinding layoutBinding)
	{
		if (std::find(s_DescriptorSetLayoutBindings.begin(), s_DescriptorSetLayoutBindings.end(), layoutBinding) != s_DescriptorSetLayoutBindings.end())
		{
			s_DescriptorSetLayoutBindings.erase(std::find(s_DescriptorSetLayoutBindings.begin(), s_DescriptorSetLayoutBindings.end(), layoutBinding));
		}
		CreateDescriptorSetLayout();
		CreateDescriptorSet();
	}
	void VulkanUniformBuffer::CreateDescriptorSet()
	{
		VkDescriptorSetAllocateInfo		allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = VulkanContext::GetDescriptorPool();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &s_DescriptorSetLayout;

		VK_CHECK_RESULT(vkAllocateDescriptorSets(VulkanContext::GetDevice(), &allocInfo, &s_DescriptorSet));
	}
	void VulkanUniformBuffer::CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(s_DescriptorSetLayoutBindings.size());
		layoutInfo.pBindings = s_DescriptorSetLayoutBindings.data();

		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(VulkanContext::GetDevice(), &layoutInfo, nullptr, &s_DescriptorSetLayout));
	}
}