#pragma once
#include "GEngine/Compute/StorageBuffer.h"
#include <vulkan/vulkan.h>

namespace GEngine
{
	class GENGINE_API VulkanStorageBuffer : public StorageBuffer
	{
	public:
		VulkanStorageBuffer(uint32_t size);
		virtual ~VulkanStorageBuffer();
		virtual void SetData(uint32_t size, const void* data, uint32_t offset = 0) override;
		virtual void Bind(uint32_t slot) override;

		VkDescriptorSetLayoutBinding GetDescriptorSetLayoutBinding() { return m_DescriptorSetLayoutBinding; }
		VkDescriptorBufferInfo GetDescriptorBufferInfo() { return m_BufferInfo; }
	private:
		VkBuffer		m_StorageBuffer;
		VkDeviceMemory	m_StorageBufferMemory;
		uint32_t					m_Offset = 0;
		VkDescriptorBufferInfo		m_BufferInfo{};
		VkDescriptorSetLayoutBinding m_DescriptorSetLayoutBinding;
	};
}


