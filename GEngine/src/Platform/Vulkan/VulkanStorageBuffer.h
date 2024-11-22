#pragma once
#include "GEngine/Compute/StorageBuffer.h"
#include <vulkan/vulkan.h>

namespace GEngine
{
	class GENGINE_API VulkanStorageBuffer : public StorageBuffer
	{
	public:
		VulkanStorageBuffer(uint32_t size, uint32_t binding);
		virtual void SetData(uint32_t size, const void* data, uint32_t offset = 0) override;
	private:
		VkBuffer		m_StorageBuffer;
		VkDeviceMemory	m_StorageBufferMemory;
		uint32_t		m_Binding;
		uint32_t					m_Offset = 0;
		VkDescriptorBufferInfo		m_BufferInfo{};
		VkDescriptorSetLayoutBinding m_DescriptorSetLayoutBinding;
	};
}


