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
		virtual void ReadData(uint32_t size, void* destination, uint32_t offset = 0) override;
		virtual void Bind(uint32_t slot) override;

		VkDescriptorBufferInfo* GetDescriptorBufferInfo() { return &m_BufferInfo; }
		virtual void* GetNativeResource() const override { return reinterpret_cast<void*>(m_StorageBuffer); }

		bool operator==(const VulkanStorageBuffer& other) const
		{
			return m_StorageBuffer == other.m_StorageBuffer && m_StorageBufferMemory == other.m_StorageBufferMemory;
		}
	private:
		uint32_t m_Size = 0;
		VkBuffer					m_StorageBuffer = VK_NULL_HANDLE;
		VkDeviceMemory				m_StorageBufferMemory = VK_NULL_HANDLE;
		uint32_t					m_Offset = 0;
		VkDescriptorBufferInfo		m_BufferInfo{};
	};
}


