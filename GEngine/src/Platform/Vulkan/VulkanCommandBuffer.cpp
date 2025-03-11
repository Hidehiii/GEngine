#include "GEpch.h"
#include "VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanComputePipeline.h"

namespace GEngine
{
	VulkanCommandBufferPool::VulkanCommandBufferPool(QueueFamilyIndices queueFamilyIndices, int count)
	{
		CreateCommandPool(queueFamilyIndices);
		m_GraphicsCommandBuffers.resize(count);
		m_ComputeCommandBuffers.resize(count);
		m_SecondaryCommandBuffers.resize(count);

		VkCommandBufferAllocateInfo		allocInfo{};
		allocInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool			= m_GraphicsCommandPool;
		allocInfo.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount	= count;

		VK_CHECK_RESULT(vkAllocateCommandBuffers(VulkanContext::Get()->GetDevice(), &allocInfo, m_GraphicsCommandBuffers.data()));

		allocInfo.commandPool			= m_ComputeCommandPool;

		VK_CHECK_RESULT(vkAllocateCommandBuffers(VulkanContext::Get()->GetDevice(), &allocInfo, m_ComputeCommandBuffers.data()));


		allocInfo.level					= VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		VK_CHECK_RESULT(vkAllocateCommandBuffers(VulkanContext::Get()->GetDevice(), &allocInfo, m_SecondaryCommandBuffers.data()));
	}
	VulkanCommandBufferPool::~VulkanCommandBufferPool()
	{
		
	}
	void VulkanCommandBufferPool::Release()
	{
		if (VulkanContext::Get()->GetDevice())
		{
			if (m_GraphicsCommandPool != VK_NULL_HANDLE)
			{
				vkDestroyCommandPool(VulkanContext::Get()->GetDevice(), m_GraphicsCommandPool, nullptr);
			}
			if (m_ComputeCommandPool != VK_NULL_HANDLE)
			{
				vkDestroyCommandPool(VulkanContext::Get()->GetDevice(), m_ComputeCommandPool, nullptr);
			}
		}
		
			
	}
	VkCommandBuffer VulkanCommandBufferPool::BeginSingleTimeGraphicsCommands()
	{
		VkCommandBufferAllocateInfo			allocInfo{};
		allocInfo.sType						= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool				= m_GraphicsCommandPool;
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
	void VulkanCommandBufferPool::EndSingleTimeGraphicsCommands(VkCommandBuffer commandBuffer, VkQueue queue)
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

		vkFreeCommandBuffers(VulkanContext::Get()->GetDevice(), m_GraphicsCommandPool, 1, &commandBuffer);
		vkDestroyFence(VulkanContext::Get()->GetDevice(), fence, nullptr);
	}
	void VulkanCommandBufferPool::CreateCommandPool(QueueFamilyIndices queueFamilyIndices)
	{
		VkCommandPoolCreateInfo		poolInfo{};
		poolInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags				= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex	= queueFamilyIndices.GraphicsFamily.value();

		VK_CHECK_RESULT(vkCreateCommandPool(VulkanContext::Get()->GetDevice(), &poolInfo, nullptr, &m_GraphicsCommandPool));

		poolInfo.queueFamilyIndex	= queueFamilyIndices.ComputeFamily.value();

		VK_CHECK_RESULT(vkCreateCommandPool(VulkanContext::Get()->GetDevice(), &poolInfo, nullptr, &m_ComputeCommandPool));
	}

	VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer buffer, CommandBufferType type, VkSemaphore semaphore ,VkFence fence)
	{
		m_CommandBuffer = buffer;
		m_Semaphore		= semaphore;
		m_Fence			= fence;
		m_Type			= type;
	}
	VulkanCommandBuffer::~VulkanCommandBuffer()
	{

	}
	void VulkanCommandBuffer::Begin(Ref<FrameBuffer>& buffer, const Editor::EditorCamera& camera)
	{
		m_FrameBuffer = std::static_pointer_cast<VulkanFrameBuffer>(buffer);
		m_FrameBuffer->Begin(this);
	}
	void VulkanCommandBuffer::Begin(Ref<FrameBuffer>& buffer, const Camera& camera)
	{
		m_FrameBuffer = std::static_pointer_cast<VulkanFrameBuffer>(buffer);
		m_FrameBuffer->Begin(this);
	}
	void VulkanCommandBuffer::End()
	{
		m_FrameBuffer->End(this);
	}
	void VulkanCommandBuffer::Render(Ref<Scene>& scene)
	{
	}
	void VulkanCommandBuffer::Render(Ref<GraphicsPipeline>& pipeline, uint32_t instanceCount, uint32_t indexCount)
	{
		std::static_pointer_cast<VulkanGraphicsPipeline>(pipeline)->Render(this, m_FrameBuffer, instanceCount, indexCount);
	}
	void VulkanCommandBuffer::Compute(Ref<ComputePipeline>& pipeline, uint32_t x, uint32_t y, uint32_t z)
	{
		std::static_pointer_cast<VulkanComputePipeline>(pipeline)->Compute(this, x, y, z);
	}
}