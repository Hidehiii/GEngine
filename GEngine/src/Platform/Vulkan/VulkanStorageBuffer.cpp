#include "GEpch.h"
#include "Platform/Vulkan/VulkanStorageBuffer.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace GEngine
{
	void VulkanStorageBuffer::ReadData(uint32_t size, void* destination, uint32_t offset)
	{
		if (m_ExclusiveSharing && m_QueueOwnership != 0)
			throw std::invalid_argument("Exclusive Vulkan storage readback requires graphics ownership.");
		if (!destination || !size || offset > m_Size || size > m_Size - offset)
			throw std::invalid_argument("Storage readback exceeds buffer bounds.");
		auto* context = VulkanContext::Get();
		context->WaitForIdle();
		VkBuffer staging = VK_NULL_HANDLE;
		VmaAllocation stagingAllocation = nullptr;
		try
		{
			Utils::CreateBuffer(context->GetPhysicalDevice(), context->GetDevice(), size, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging, stagingAllocation);
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
			if (vmaMapMemory(context->GetVmaAllocator(), stagingAllocation, &mapped) != VK_SUCCESS)
				throw std::runtime_error("Failed to map Vulkan storage readback memory.");
			memcpy(destination, mapped, size);
			vmaUnmapMemory(context->GetVmaAllocator(), stagingAllocation);
		}
		catch (...)
		{
			if (staging) vmaDestroyBuffer(context->GetVmaAllocator(), staging, stagingAllocation);
			throw;
		}
		vmaDestroyBuffer(context->GetVmaAllocator(), staging, stagingAllocation);
	}
	VulkanStorageBuffer::VulkanStorageBuffer(uint32_t size) : m_Size(size)
	{
		m_ExclusiveSharing = Utils::UseExclusiveComputeBufferSharing();
		Utils::CreateBuffer(VulkanContext::Get()->GetPhysicalDevice(),
			VulkanContext::Get()->GetDevice(),
			size,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_StorageBuffer,
			m_StorageBufferAllocation, m_ExclusiveSharing);

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
			const VmaAllocation storageAllocation = m_StorageBufferAllocation;
			const VmaAllocator allocator = context->GetVmaAllocator();
			context->RetireResource([storageBuffer, storageAllocation, allocator](VkDevice device)
			{
				if (storageBuffer != VK_NULL_HANDLE)
					vmaDestroyBuffer(allocator, storageBuffer, storageAllocation);
			});
		}
		m_StorageBuffer = VK_NULL_HANDLE;
		m_StorageBufferAllocation = nullptr;
	}
	void VulkanStorageBuffer::SetData(uint32_t size, const void* data, uint32_t offset)
	{
		if (m_ExclusiveSharing && m_QueueOwnership != 0)
			throw std::invalid_argument("Exclusive Vulkan storage upload requires graphics ownership.");
		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VmaAllocation stagingBufferAllocation = nullptr;
		Utils::CreateBuffer(VulkanContext::Get()->GetPhysicalDevice(),
			VulkanContext::Get()->GetDevice(),
			size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferAllocation);

		void* tempData = nullptr;
		vmaMapMemory(VulkanContext::Get()->GetVmaAllocator(), stagingBufferAllocation, &tempData);
		memcpy(tempData, data, size);
		vmaUnmapMemory(VulkanContext::Get()->GetVmaAllocator(), stagingBufferAllocation);
		Utils::CopyBufferToBuffer(stagingBuffer, m_StorageBuffer, size);

		vmaDestroyBuffer(VulkanContext::Get()->GetVmaAllocator(), stagingBuffer, stagingBufferAllocation);
	}
	void VulkanStorageBuffer::Bind(uint32_t slot)
	{
	}
}

