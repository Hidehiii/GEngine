#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/UniformBuffer.h"
#include <vulkan/vulkan.h>

namespace GEngine
{
	class GENGINE_API VulkanUniformBuffer : public UniformBuffer
	{
	public:
		VulkanUniformBuffer(uint32_t size, uint32_t binding);
		virtual ~VulkanUniformBuffer();

		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

		VkDescriptorSetLayoutBinding GetDescriptorSetLayoutBinding() { return m_DescriptorSetLayoutBinding; }
		VkDescriptorBufferInfo* GetDescriptorBufferInfo() { return &m_BufferInfo; }
	private:
		VkBuffer					m_UniformBuffer;
		VkDeviceMemory				m_UniformBufferMemory;
		void*						m_MapData;
		VkDescriptorBufferInfo		m_BufferInfo{};
		VkDescriptorSetLayoutBinding m_DescriptorSetLayoutBinding;
	};
}



