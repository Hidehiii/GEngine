#include "GEpch.h"
#include "Platform/Vulkan/VulkanStorageBuffer.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace GEngine
{
	VulkanStorageBuffer::VulkanStorageBuffer(uint32_t size)
	{

		Utils::CreateBuffer(VulkanContext::Get()->GetPhysicalDevice(),
			VulkanContext::Get()->GetDevice(),
			size,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_StorageBuffer,
			m_StorageBufferMemory);

		m_BufferInfo.buffer = m_StorageBuffer;
		m_BufferInfo.offset = m_Offset;
		m_BufferInfo.range = size;
	}
	VulkanStorageBuffer::~VulkanStorageBuffer()
	{
		auto* context = VulkanContext::Get();
		if (context != nullptr && context->GetDevice() != VK_NULL_HANDLE)
		{
			const VkBuffer storageBuffer = m_StorageBuffer;
			const VkDeviceMemory storageMemory = m_StorageBufferMemory;
			context->RetireResource([storageBuffer, storageMemory](VkDevice device)
			{
				if (storageBuffer != VK_NULL_HANDLE)
					vkDestroyBuffer(device, storageBuffer, nullptr);
				if (storageMemory != VK_NULL_HANDLE)
					vkFreeMemory(device, storageMemory, nullptr);
			});
		}
		m_StorageBuffer = VK_NULL_HANDLE;
		m_StorageBufferMemory = VK_NULL_HANDLE;
	}
	void VulkanStorageBuffer::SetData(uint32_t size, const void* data, uint32_t offset)
	{
		VkBuffer	stagingBuffer;
		VkDeviceMemory	stagingBufferMemory;
		Utils::CreateBuffer(VulkanContext::Get()->GetPhysicalDevice(),
			VulkanContext::Get()->GetDevice(),
			size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		void* tempData;
		vkMapMemory(VulkanContext::Get()->GetDevice(), stagingBufferMemory, offset, size, 0, &tempData);
		memcpy(tempData, data, size);
		vkUnmapMemory(VulkanContext::Get()->GetDevice(), stagingBufferMemory);
		Utils::CopyBufferToBuffer(stagingBuffer, m_StorageBuffer, size);

		vkDestroyBuffer(VulkanContext::Get()->GetDevice(), stagingBuffer, nullptr);
		vkFreeMemory(VulkanContext::Get()->GetDevice(), stagingBufferMemory, nullptr);
	}
	void VulkanStorageBuffer::Bind(uint32_t slot)
	{
	}
}

