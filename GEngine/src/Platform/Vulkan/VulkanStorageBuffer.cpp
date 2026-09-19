#include "GEpch.h"
#include "Platform/Vulkan/VulkanStorageBuffer.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace GEngine
{
	void VulkanStorageBuffer::ReadData(uint32_t size, void* destination, uint32_t offset)
	{
		if (!destination || !size || offset > m_Size || size > m_Size - offset)
			throw std::invalid_argument("Storage readback exceeds buffer bounds.");
		auto* context = VulkanContext::Get();
		context->WaitForIdle();
		VkBuffer staging = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		try
		{
			Utils::CreateBuffer(context->GetPhysicalDevice(), context->GetDevice(), size, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging, memory);
			auto command = context->BeginSingleTimeGraphicsCommand();
			VkMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);
			VkBufferCopy region{ offset, 0, size };
			vkCmdCopyBuffer(command, m_StorageBuffer, staging, 1, &region);
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
			vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);
			context->EndSingleTimeGraphicsCommand(command);
			void* mapped = nullptr;
			if (vkMapMemory(context->GetDevice(), memory, 0, size, 0, &mapped) != VK_SUCCESS)
				throw std::runtime_error("Failed to map Vulkan storage readback memory.");
			memcpy(destination, mapped, size);
			vkUnmapMemory(context->GetDevice(), memory);
		}
		catch (...)
		{
			if (staging) vkDestroyBuffer(context->GetDevice(), staging, nullptr);
			if (memory) vkFreeMemory(context->GetDevice(), memory, nullptr);
			throw;
		}
		vkDestroyBuffer(context->GetDevice(), staging, nullptr);
		vkFreeMemory(context->GetDevice(), memory, nullptr);
	}
	VulkanStorageBuffer::VulkanStorageBuffer(uint32_t size) : m_Size(size)
	{

		Utils::CreateBuffer(VulkanContext::Get()->GetPhysicalDevice(),
			VulkanContext::Get()->GetDevice(),
			size,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
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

