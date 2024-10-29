#include "GEpch.h"
#include "VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanUtils.h"
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

			VK_CHECK_RESULT(vkAllocateCommandBuffers(VulkanContext::GetDevice(), &allocInfo, &buffer));
			m_CommandBuffers.push_back(buffer);
		}
	}
	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		Release();
	}
	void VulkanCommandBuffer::Release()
	{
		if(m_CommandPool != VK_NULL_HANDLE)
			vkDestroyCommandPool(VulkanContext::GetDevice(), m_CommandPool, nullptr);
	}
	VkCommandBuffer VulkanCommandBuffer::BeginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo			allocInfo{};
		allocInfo.sType						= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level						= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool				= m_CommandPool;
		allocInfo.commandBufferCount		= 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(VulkanContext::GetDevice(), &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}
	void VulkanCommandBuffer::EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(queue);

		vkFreeCommandBuffers(VulkanContext::GetDevice(), m_CommandPool, 1, &commandBuffer);
	}
	void VulkanCommandBuffer::CreateCommandPool(QueueFamilyIndices queueFamilyIndices)
	{
		VkCommandPoolCreateInfo		poolInfo{};
		poolInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags				= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex	= queueFamilyIndices.GraphicsFamily.value();

		VK_CHECK_RESULT(vkCreateCommandPool(VulkanContext::GetDevice(), &poolInfo, nullptr, &m_CommandPool));
	}
}