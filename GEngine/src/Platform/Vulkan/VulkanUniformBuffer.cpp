#include "GEpch.h"
#include "VulkanUniformBuffer.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanContext.h"



namespace GEngine
{

	VulkanUniformBuffer::VulkanUniformBuffer(uint32_t size, uint32_t binding)
	{
		m_Binding = binding;


		m_DescriptorSetLayoutBinding.binding			= binding;
		m_DescriptorSetLayoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
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


	VulkanUniformBufferDynamic::VulkanUniformBufferDynamic(uint32_t size, uint32_t count, uint32_t binding, uint32_t globalIndex)
	{
		m_Binding				= binding;
		uint32_t minUboAligment = Graphics::GetMinUniformBufferOffsetAlignment();
		m_Aligment				= (size + minUboAligment - 1) & ~(minUboAligment - 1);
		m_TotalSize				= m_Aligment * count;
		m_GlobalIndex			= globalIndex;

		m_DescriptorSetLayoutBinding.binding			= binding;
		m_DescriptorSetLayoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
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

		m_BufferInfo.buffer = m_UniformBuffer;
		m_BufferInfo.offset = 0;
		m_BufferInfo.range	= size;

	}
	VulkanUniformBufferDynamic::~VulkanUniformBufferDynamic()
	{
		if (VulkanContext::Get()->GetDevice())
		{
			vkDestroyBuffer(VulkanContext::Get()->GetDevice(), m_UniformBuffer, nullptr);
			vkFreeMemory(VulkanContext::Get()->GetDevice(), m_UniformBufferMemory, nullptr);
		}
	}
	void VulkanUniformBufferDynamic::SetData(const void* data, uint32_t size)
	{
		GE_CORE_ASSERT(size <= m_Aligment, "");

		m_Offset = m_Offset + m_Aligment <= m_TotalSize ? m_Offset + m_Aligment : 0;

		memcpy(((std::byte*)m_MapData) + m_Offset, data, size);

		m_BufferInfo.range = size;

		UpdateGlobalUniformOffset();
	}
}