#include "GEpch.h"
#include "VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
namespace GEngine
{
	VulkanCommandBuffer::VulkanCommandBuffer(QueueFamilyIndices queueFamilyIndices, int count)
	{
		CreateCommandPool(queueFamilyIndices);
		m_CommandBuffers.resize(count);
		m_SecondaryCommandBuffers.resize(count);

		VkCommandBufferAllocateInfo		allocInfo{};
		allocInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool			= m_CommandPool;
		allocInfo.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount	= count;

		VK_CHECK_RESULT(vkAllocateCommandBuffers(VulkanContext::Get()->GetDevice(), &allocInfo, m_CommandBuffers.data()));


		allocInfo.level					= VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		VK_CHECK_RESULT(vkAllocateCommandBuffers(VulkanContext::Get()->GetDevice(), &allocInfo, m_SecondaryCommandBuffers.data()));
	}
	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		
	}
	void VulkanCommandBuffer::Release()
	{
		if (m_CommandPool != VK_NULL_HANDLE)
		{
			if (VulkanContext::Get()->GetDevice())
			{
				vkDestroyCommandPool(VulkanContext::Get()->GetDevice(), m_CommandPool, nullptr);
			}
		}
			
	}
	VkCommandBuffer VulkanCommandBuffer::BeginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo			allocInfo{};
		allocInfo.sType						= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool				= m_CommandPool;
		allocInfo.level						= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount		= 1;

		VkCommandBuffer commandBuffer;
		VK_CHECK_RESULT(vkAllocateCommandBuffers(VulkanContext::Get()->GetDevice(), &allocInfo, &commandBuffer));

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}
	void VulkanCommandBuffer::EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue)
	{
		vkEndCommandBuffer(commandBuffer);
		// 用fence 而不是wait
		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		VkFence fence;
		VK_CHECK_RESULT(vkCreateFence(VulkanContext::Get()->GetDevice(), &fenceCreateInfo, nullptr, &fence));

		VkSubmitInfo					submitInfo{};
		submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount	= 1;
		submitInfo.pCommandBuffers		= &commandBuffer;

		vkQueueSubmit(queue, 1, &submitInfo, fence);
		vkWaitForFences(VulkanContext::Get()->GetDevice(), 1, &fence, VK_TRUE, UINT64_MAX);

		vkFreeCommandBuffers(VulkanContext::Get()->GetDevice(), m_CommandPool, 1, &commandBuffer);
		vkDestroyFence(VulkanContext::Get()->GetDevice(), fence, nullptr);
	}
	void VulkanCommandBuffer::CreateCommandPool(QueueFamilyIndices queueFamilyIndices)
	{
		VkCommandPoolCreateInfo		poolInfo{};
		poolInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags				= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex	= queueFamilyIndices.GraphicsFamily.value();

		VK_CHECK_RESULT(vkCreateCommandPool(VulkanContext::Get()->GetDevice(), &poolInfo, nullptr, &m_CommandPool));
	}
}