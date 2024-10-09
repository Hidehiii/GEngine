#include "GEpch.h"
#include "VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanContext.h"
namespace GEngine
{
	VulkanCommandBuffer::VulkanCommandBuffer(QueueFamilyIndices queueFamilyIndices, int count)
	{
		CreateCommandPool(queueFamilyIndices);
		for (int i = 0; i < count; i++)
		{
			VkCommandBuffer					buffer;
			VkCommandBufferAllocateInfo		allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = m_CommandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = 1;

			if (vkAllocateCommandBuffers(VulkanContext::GetDevice(), &allocInfo, &buffer) != VK_SUCCESS) {
				GE_CORE_ERROR("failed to allocate command buffers!");
			}
			m_CommandBuffers.push_back(buffer);
		}
	}
	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		vkDestroyCommandPool(VulkanContext::GetDevice(), m_CommandPool, nullptr);
	}
	void VulkanCommandBuffer::CreateCommandPool(QueueFamilyIndices queueFamilyIndices)
	{
		VkCommandPoolCreateInfo		poolInfo{};
		poolInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags				= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex	= queueFamilyIndices.GraphicsFamily.value();

		if (vkCreateCommandPool(VulkanContext::GetDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
			GE_CORE_ERROR("failed to create command pool!");
		}
	}
}