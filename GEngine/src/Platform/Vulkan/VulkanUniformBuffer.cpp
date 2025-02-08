#include "GEpch.h"
#include "VulkanUniformBuffer.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanContext.h"



namespace GEngine
{
	std::vector<VulkanUniformBuffer*>	VulkanUniformBuffer::s_PublicUniformBuffer;

	VulkanUniformBuffer::VulkanUniformBuffer(uint32_t size, uint32_t binding)
	{
		// 公共ubo索引不为0
		if (binding != 0)
		{
			AddPublicUniformBuffer(this);
		}

		m_DescriptorSetLayoutBinding.binding			= binding;
		m_DescriptorSetLayoutBinding.descriptorType		= binding != 0 ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		//m_DescriptorSetLayoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		m_DescriptorSetLayoutBinding.descriptorCount	= 1;
		m_DescriptorSetLayoutBinding.stageFlags			= VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT;
		m_DescriptorSetLayoutBinding.pImmutableSamplers = nullptr; // Optional
		
		Utils::CreateBuffer(VulkanContext::Get()->GetPhysicalDevice(), 
							VulkanContext::Get()->GetDevice(), 
							size, 
							VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
							m_UniformBuffer, 
							m_UniformBufferMemory);
		vkMapMemory(VulkanContext::Get()->GetDevice(), m_UniformBufferMemory, 0, size, 0, &m_MapData);

		m_BufferInfo.buffer		= m_UniformBuffer;
		m_BufferInfo.offset		= 0;
		m_BufferInfo.range		= size;
		
	}
	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		if (VulkanContext::Get()->GetDevice())
		{
			vkDestroyBuffer(VulkanContext::Get()->GetDevice(), m_UniformBuffer, nullptr);
			vkFreeMemory(VulkanContext::Get()->GetDevice(), m_UniformBufferMemory, nullptr);
		}
		
	}
	void VulkanUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		memcpy(((std::byte*)m_MapData) + offset, data, size);
		//memcpy(m_MapData, data, size);

		m_BufferInfo.range = size;
	}
	void VulkanUniformBuffer::AddPublicUniformBuffer(VulkanUniformBuffer* buffer)
	{
		s_PublicUniformBuffer.push_back(buffer);
	}
	void VulkanUniformBuffer::RemovePublicUniformBuffer(VulkanUniformBuffer* buffer)
	{
		if (std::find(s_PublicUniformBuffer.begin(), s_PublicUniformBuffer.end(), buffer) != s_PublicUniformBuffer.end())
		{
			s_PublicUniformBuffer.erase(std::find(s_PublicUniformBuffer.begin(), s_PublicUniformBuffer.end(), buffer));
		}
	}
}