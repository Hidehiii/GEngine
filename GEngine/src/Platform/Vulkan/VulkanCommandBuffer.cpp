#include "GEpch.h"
#include "VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanComputePipeline.h"
#include "GEngine/Graphics/Graphics.h"

namespace GEngine
{
	VulkanCommandBufferPool::VulkanCommandBufferPool(QueueFamilyIndices queueFamilyIndices, int count)
	{
		CreateCommandPool(queueFamilyIndices);
		m_GraphicsCommandBuffers.resize(count);
		m_ComputeCommandBuffers.resize(count);
		m_TransferCommandBuffers.resize(count);
		m_SecondaryCommandBuffers.resize(count);

		VkCommandBufferAllocateInfo		allocInfo{};
		allocInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool			= m_GraphicsCommandPool;
		allocInfo.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount	= count;

		VK_CHECK_RESULT(vkAllocateCommandBuffers(VulkanContext::Get()->GetDevice(), &allocInfo, m_GraphicsCommandBuffers.data()));

		allocInfo.commandPool			= m_ComputeCommandPool;

		VK_CHECK_RESULT(vkAllocateCommandBuffers(VulkanContext::Get()->GetDevice(), &allocInfo, m_ComputeCommandBuffers.data()));

		allocInfo.commandPool			= m_TransferCommandPool;

		VK_CHECK_RESULT(vkAllocateCommandBuffers(VulkanContext::Get()->GetDevice(), &allocInfo, m_TransferCommandBuffers.data()));

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
			if (m_TransferCommandPool != VK_NULL_HANDLE)
			{
				vkDestroyCommandPool(VulkanContext::Get()->GetDevice(), m_TransferCommandPool, nullptr);
			}
		}
		
			
	}
	VkCommandBuffer VulkanCommandBufferPool::BeginSingleTimeGraphicsCommand()
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
	void VulkanCommandBufferPool::EndSingleTimeGraphicsCommand(VkCommandBuffer commandBuffer)
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

		vkQueueSubmit(VulkanContext::Get()->GetGraphicsQueue(), 1, &submitInfo, fence);
		vkWaitForFences(VulkanContext::Get()->GetDevice(), 1, &fence, VK_TRUE, UINT64_MAX);

		vkFreeCommandBuffers(VulkanContext::Get()->GetDevice(), m_GraphicsCommandPool, 1, &commandBuffer);
		vkDestroyFence(VulkanContext::Get()->GetDevice(), fence, nullptr);
	}
	VkCommandBuffer VulkanCommandBufferPool::BeginSingleTimeComputeCommand()
	{
		VkCommandBufferAllocateInfo			allocInfo{};
		allocInfo.sType						= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool				= m_ComputeCommandPool;
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
	void VulkanCommandBufferPool::EndSingleTimeComputeCommand(VkCommandBuffer commandBuffer)
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

		vkQueueSubmit(VulkanContext::Get()->GetComputeQueue(), 1, &submitInfo, fence);
		vkWaitForFences(VulkanContext::Get()->GetDevice(), 1, &fence, VK_TRUE, UINT64_MAX);

		vkFreeCommandBuffers(VulkanContext::Get()->GetDevice(), m_ComputeCommandPool, 1, &commandBuffer);
		vkDestroyFence(VulkanContext::Get()->GetDevice(), fence, nullptr);
	}
	VkCommandBuffer VulkanCommandBufferPool::BeginSingleTimeTransferCommand()
	{
		VkCommandBufferAllocateInfo			allocInfo{};
		allocInfo.sType						= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool				= m_TransferCommandPool;
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
	void VulkanCommandBufferPool::EndSingleTimeTransferCommand(VkCommandBuffer commandBuffer)
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

		vkQueueSubmit(VulkanContext::Get()->GetTransferQueue(), 1, &submitInfo, fence);
		vkWaitForFences(VulkanContext::Get()->GetDevice(), 1, &fence, VK_TRUE, UINT64_MAX);

		vkFreeCommandBuffers(VulkanContext::Get()->GetDevice(), m_TransferCommandPool, 1, &commandBuffer);
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

		poolInfo.queueFamilyIndex	= queueFamilyIndices.TransferFamily.value();

		VK_CHECK_RESULT(vkCreateCommandPool(VulkanContext::Get()->GetDevice(), &poolInfo, nullptr, &m_TransferCommandPool));
	}

	VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer buffer, CommandBufferType type)
	{
		m_CommandBuffer = buffer;
		m_Type			= type;
	}
	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
	}
	void VulkanCommandBuffer::Begin(Ref<FrameBuffer>& buffer)
	{
		VkCommandBufferBeginInfo    beginInfo{};
		beginInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags				= 0; // Optional
		beginInfo.pInheritanceInfo	= nullptr; // Optional

		vkResetCommandBuffer(m_CommandBuffer, 0);
		VK_CHECK_RESULT(vkBeginCommandBuffer(m_CommandBuffer, &beginInfo));

		
		if (m_Type == CommandBufferType::Graphics)
		{
			GE_CORE_ASSERT(buffer != nullptr, "graphics cmd must have frame buffer");
			m_FrameBuffer = std::static_pointer_cast<VulkanFrameBuffer>(buffer);
			m_FrameBuffer->Begin(this);
		}

		if(buffer != nullptr)
			Graphics::UpdateScreenUniform(Vector4{ buffer->GetWidth(), buffer->GetHeight(), 0, 0 });
	}
	Ref<VulkanCommandBuffer> VulkanCommandBuffer::Create(VkCommandBuffer buffer, CommandBufferType type)
	{
		return CreateRef<VulkanCommandBuffer>(buffer, type);
	}
	void VulkanCommandBuffer::End()
	{
		if (m_Type == CommandBufferType::Graphics)
		{
			m_FrameBuffer->End(this);
		}
		VK_CHECK_RESULT(vkEndCommandBuffer(m_CommandBuffer));

		std::vector<VkPipelineStageFlags> waitStages;
		
		if (m_Type == CommandBufferType::Graphics)
			waitStages = std::vector<VkPipelineStageFlags>(m_WaitSemaphores.size(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		if(m_Type == CommandBufferType::Compute)
			waitStages = std::vector<VkPipelineStageFlags>(m_WaitSemaphores.size(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		if(m_Type == CommandBufferType::Transfer)
			waitStages = std::vector<VkPipelineStageFlags>(m_WaitSemaphores.size(), VK_PIPELINE_STAGE_TRANSFER_BIT);

		VkSubmitInfo                    submitInfo{};
		submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount	= 1;
		submitInfo.pCommandBuffers		= &m_CommandBuffer;
		submitInfo.waitSemaphoreCount	= m_WaitSemaphores.size();
		submitInfo.pWaitSemaphores		= m_WaitSemaphores.data();
		submitInfo.pWaitDstStageMask	= waitStages.data();
		submitInfo.signalSemaphoreCount = m_SignalSemaphores.size();
		submitInfo.pSignalSemaphores	= m_SignalSemaphores.data();

		if(m_Type == CommandBufferType::Graphics)
			VK_CHECK_RESULT(vkQueueSubmit(VulkanContext::Get()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
		if(m_Type == CommandBufferType::Compute)
			VK_CHECK_RESULT(vkQueueSubmit(VulkanContext::Get()->GetComputeQueue(), 1, &submitInfo, VK_NULL_HANDLE));
		if (m_Type == CommandBufferType::Transfer)
			VK_CHECK_RESULT(vkQueueSubmit(VulkanContext::Get()->GetTransferQueue(), 1, &submitInfo, VK_NULL_HANDLE));
		
		ClearSignalSemaphores();
		ClearWaitSemaphores();
	}
	void VulkanCommandBuffer::Render(Ref<GraphicsPipeline>& pipeline, std::string pass, uint32_t instanceCount, uint32_t indexCount)
	{
		std::static_pointer_cast<VulkanGraphicsPipeline>(pipeline)->Render(this, m_FrameBuffer, pass, instanceCount, indexCount);
	}
	void VulkanCommandBuffer::Compute(Ref<ComputePipeline>& pipeline, std::string pass, uint32_t x, uint32_t y, uint32_t z)
	{
		std::static_pointer_cast<VulkanComputePipeline>(pipeline)->Compute(this, pass, x, y, z);
	}
}