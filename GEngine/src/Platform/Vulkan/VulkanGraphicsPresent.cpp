#include "GEpch.h"
#include "VulkanGraphicsPresent.h"
#include "GEngine/Application.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "GEngine/Renderer/RenderCommand.h"
#include "GEngine/Renderer/Renderer.h"

namespace GEngine
{
	VulkanGraphicsPresent::VulkanGraphicsPresent()
	{
		m_Semaphores.resize(Renderer::GetFramesInFlight());
		m_Fences.resize(Renderer::GetFramesInFlight());
		s_CommandBuffers.resize(Renderer::GetFramesInFlight());
		for (int i = 0; i < m_Semaphores.size(); i++)
		{
			VkSemaphoreCreateInfo   semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			VK_CHECK_RESULT(vkCreateSemaphore(VulkanContext::Get()->GetDevice(), &semaphoreInfo, nullptr, &m_Semaphores[i]));
		}
		for (int i = 0; i < m_Fences.size(); i++)
		{
			VkFenceCreateInfo       fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			VK_CHECK_RESULT(vkCreateFence(VulkanContext::Get()->GetDevice(), &fenceInfo, nullptr, &m_Fences[i]));
		}

		std::vector<VkCommandBuffer>	cmds;
		cmds.resize(Renderer::GetFramesInFlight());

		VkCommandBufferAllocateInfo		allocInfo{};
		allocInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool			= VulkanContext::Get()->GetGraphicsCommandPool();
		allocInfo.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount	= Renderer::GetFramesInFlight();
		VK_CHECK_RESULT(vkAllocateCommandBuffers(VulkanContext::Get()->GetDevice(), &allocInfo, cmds.data()));

		for (int i = 0; i < s_CommandBuffers.size(); i++)
		{
			s_CommandBuffers.at(i) = VulkanCommandBuffer::Create(cmds.at(i), CommandBufferType::Graphics, m_Semaphores.at(i), m_Fences.at(i));
		}
	}
	bool VulkanGraphicsPresent::AquireImage()
	{
		VK_CHECK_RESULT(vkWaitForFences(VulkanContext::Get()->GetDevice(), 1, &VulkanContext::Get()->GetCurrentFence(), VK_TRUE, std::numeric_limits<uint64_t>::max()));
		VkResult result = vkAcquireNextImageKHR(VulkanContext::Get()->GetDevice(),
			VulkanContext::Get()->GetSwapChain(),
			std::numeric_limits<uint64_t>::max(),
			VulkanContext::Get()->GetCurrentSemaphore(),
			VK_NULL_HANDLE,
			&m_SwapChainImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR ||
			(m_WindowResize.x != 0 && m_WindowResize.y != 0))
		{
			VulkanContext::Get()->RecreateSwapChain(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
			m_WindowResize.x = 0;
			m_WindowResize.y = 0;
			VkSemaphore submitWaitSemaphores[] = { VulkanContext::Get()->GetCurrentSemaphore() };
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			VkSubmitInfo					submitInfo{};
			submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount	= 0;
			submitInfo.pCommandBuffers		= nullptr;
			submitInfo.waitSemaphoreCount	= 1;
			submitInfo.pWaitSemaphores		= submitWaitSemaphores;
			submitInfo.pWaitDstStageMask	= waitStages;
			submitInfo.signalSemaphoreCount = 0;
			submitInfo.pSignalSemaphores	= nullptr;
			VK_CHECK_RESULT(vkQueueSubmit(VulkanContext::Get()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
			VulkanContext::Get()->MoveToNextSemaphore();
			GE_CORE_INFO("Recreate swapChain");
			return false;
		}

#if 0
		VK_CHECK_RESULT(vkWaitForFences(VulkanContext::Get()->GetDevice(), 1, &m_Fences.at(Renderer::GetCurrentFrame()), VK_TRUE, std::numeric_limits<uint64_t>::max()));
		VkResult result = vkAcquireNextImageKHR(VulkanContext::Get()->GetDevice(),
			VulkanContext::Get()->GetSwapChain(),
			std::numeric_limits<uint64_t>::max(),
			VK_NULL_HANDLE,
			VK_NULL_HANDLE,
			&m_SwapChainImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR ||
			(m_WindowResize.x != 0 && m_WindowResize.y != 0))
		{
			VulkanContext::Get()->RecreateSwapChain(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
			m_WindowResize.x = 0;
			m_WindowResize.y = 0;
			GE_CORE_INFO("Recreate swapChain");
			return false;
		}
		ClearWaitCommands();
#endif 

		return true;
	}
	void VulkanGraphicsPresent::Begin()
	{

		vkResetFences(VulkanContext::Get()->GetDevice(), 1, &VulkanContext::Get()->GetCurrentFence());
		VkCommandBufferBeginInfo    beginInfo{};
		beginInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags				= 0; // Optional
		beginInfo.pInheritanceInfo	= nullptr; // Optional
		VulkanContext::Get()->BeginGraphicsCommandBuffer();
		vkResetCommandBuffer(VulkanContext::Get()->GetCurrentCommandBuffer(), 0);
		VK_CHECK_RESULT(vkBeginCommandBuffer(VulkanContext::Get()->GetCurrentCommandBuffer(), &beginInfo));
		VulkanContext::Get()->GetFrameBuffer(m_SwapChainImageIndex)->Begin();
#if 0
		vkResetFences(VulkanContext::Get()->GetDevice(), 1, &m_Fences.at(Renderer::GetCurrentFrame()));
		VkCommandBufferBeginInfo    beginInfo{};
		beginInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags				= 0; // Optional
		beginInfo.pInheritanceInfo	= nullptr; // Optional
		vkResetCommandBuffer(std::dynamic_pointer_cast<VulkanCommandBuffer>(s_CommandBuffers.at(Renderer::GetCurrentFrame()))->GetCommandBuffer(), 0);
		VK_CHECK_RESULT(vkBeginCommandBuffer(std::dynamic_pointer_cast<VulkanCommandBuffer>(s_CommandBuffers.at(Renderer::GetCurrentFrame()))->GetCommandBuffer(), &beginInfo));
		VulkanContext::Get()->GetFrameBuffer(m_SwapChainImageIndex)->Begin();
#endif
	}
	void VulkanGraphicsPresent::End()
	{
		VulkanContext::Get()->GetFrameBuffer(m_SwapChainImageIndex)->End();
		VkCommandBuffer commandBuffer = VulkanContext::Get()->EndGraphicsCommandBuffer();
		VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

		VkSemaphore submitWaitSemaphores[] = { VulkanContext::Get()->GetCurrentSemaphore() };
		VulkanContext::Get()->MoveToNextSemaphore();
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore signalSemaphores[] = { VulkanContext::Get()->GetCurrentSemaphore() };

		VkSubmitInfo					submitInfo{};
		submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount	= 1;
		submitInfo.pCommandBuffers		= &commandBuffer;
		submitInfo.waitSemaphoreCount	= 1;
		submitInfo.pWaitSemaphores		= submitWaitSemaphores;
		submitInfo.pWaitDstStageMask	= waitStages;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores	= signalSemaphores;

		VK_CHECK_RESULT(vkQueueSubmit(VulkanContext::Get()->GetGraphicsQueue(), 1, &submitInfo, VulkanContext::Get()->GetCurrentFence()));
		VkSwapchainKHR swapChains[] = { VulkanContext::Get()->GetSwapChain() };

		VkSemaphore presentWaitSemaphores[] = { VulkanContext::Get()->GetCurrentSemaphore() };

		VkPresentInfoKHR			presentInfo{};
		presentInfo.sType			= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pImageIndices	= &m_SwapChainImageIndex;
		presentInfo.swapchainCount	= 1;
		presentInfo.pSwapchains		= swapChains;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = presentWaitSemaphores;
		VK_CHECK_RESULT(vkQueuePresentKHR(VulkanContext::Get()->GetPresentQueue(), &presentInfo));
		VulkanContext::Get()->MoveToNextSemaphore();

#if 0
		VulkanContext::Get()->GetFrameBuffer(m_SwapChainImageIndex)->End();
		VK_CHECK_RESULT(vkEndCommandBuffer(std::dynamic_pointer_cast<VulkanCommandBuffer>(s_CommandBuffers.at(Renderer::GetCurrentFrame()))->GetCommandBuffer()));
		std::vector<VkSemaphore> submitWaitSemaphores;
		for (auto it = s_WaitCommands.begin(); it != s_WaitCommands.end(); it++)
		{
			submitWaitSemaphores.push_back(std::dynamic_pointer_cast<VulkanCommandBuffer>(*it)->GetSemaphore());
		}
		std::vector<VkPipelineStageFlags> waitStages(submitWaitSemaphores.size(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		VkSemaphore signalSemaphores[] = { m_Semaphores.at(Renderer::GetCurrentFrame()) };

		VkSubmitInfo					submitInfo{};
		submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount	= 1;
		submitInfo.pCommandBuffers		= &commandBuffer;
		submitInfo.waitSemaphoreCount	= 1;
		submitInfo.pWaitSemaphores		= submitWaitSemaphores.data();
		submitInfo.pWaitDstStageMask	= waitStages;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores	= signalSemaphores;

		VK_CHECK_RESULT(vkQueueSubmit(VulkanContext::Get()->GetGraphicsQueue(), 1, &submitInfo, m_Fences.at(Renderer::GetCurrentFrame())));
		VkSwapchainKHR swapChains[] = { VulkanContext::Get()->GetSwapChain() };

		VkSemaphore presentWaitSemaphores[] = { m_Semaphores.at(Renderer::GetCurrentFrame()) };

		VkPresentInfoKHR			presentInfo{};
		presentInfo.sType			= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pImageIndices	= &m_SwapChainImageIndex;
		presentInfo.swapchainCount	= 1;
		presentInfo.pSwapchains		= swapChains;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = presentWaitSemaphores;
		VK_CHECK_RESULT(vkQueuePresentKHR(VulkanContext::Get()->GetPresentQueue(), &presentInfo));
#endif
	}
}