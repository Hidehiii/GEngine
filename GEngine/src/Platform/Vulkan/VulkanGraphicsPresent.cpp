#include "GEpch.h"
#include "VulkanGraphicsPresent.h"
#include "GEngine/Application.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "GEngine/Renderer/RenderCommand.h"
#include "GEngine/Renderer/Renderer.h"

namespace GEngine
{
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
		return true;
	}
	void VulkanGraphicsPresent::Begin()
	{
		ClearWaitCommands();

		vkResetFences(VulkanContext::Get()->GetDevice(), 1, &VulkanContext::Get()->GetCurrentFence());
		VkCommandBufferBeginInfo    beginInfo{};
		beginInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags				= 0; // Optional
		beginInfo.pInheritanceInfo	= nullptr; // Optional
		VulkanContext::Get()->BeginGraphicsCommandBuffer();
		vkResetCommandBuffer(VulkanContext::Get()->GetCurrentCommandBuffer(), 0);
		VK_CHECK_RESULT(vkBeginCommandBuffer(VulkanContext::Get()->GetCurrentCommandBuffer(), &beginInfo));
		VulkanContext::Get()->GetFrameBuffer(m_SwapChainImageIndex)->Begin();
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

#if 1
		std::vector<VkSemaphore> submitWaitSemaphores;
		for (auto it = s_WaitCommands.begin(); it != s_WaitCommands.end(); it++)
		{
			submitWaitSemaphores.push_back(((VulkanCommandBuffer*)*it)->GetSemaphore());
		}
		std::vector<VkPipelineStageFlags> waitStages(submitWaitSemaphores.size(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		VkSemaphore signalSemaphores[] = { VulkanContext::Get()->GetCurrentSemaphore() };
#endif

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
	}
}