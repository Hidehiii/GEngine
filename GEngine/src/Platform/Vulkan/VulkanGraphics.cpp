#include "GEpch.h"
#include "VulkanGraphics.h"
#include "GEngine/Application.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "GEngine/Renderer/RenderCommand.h"

namespace GEngine
{
	void VulkanGraphics::Begin()
	{
		vkWaitForFences(VulkanContext::Get()->GetDevice(), 1, &VulkanContext::Get()->GetInFlightFences(0), VK_TRUE, std::numeric_limits<uint64_t>::max());
		vkResetFences(VulkanContext::Get()->GetDevice(), 1, &VulkanContext::Get()->GetInFlightFences(0));
		vkAcquireNextImageKHR(VulkanContext::Get()->GetDevice(), VulkanContext::Get()->GetSwapChain(), std::numeric_limits<uint64_t>::max(), VulkanContext::Get()->GetImageAvailableSemaphores(0), VK_NULL_HANDLE, &m_SwapChainImageIndex);
		RenderCommand::BeginCommand();
		VulkanContext::Get()->GetFrameBuffer(m_SwapChainImageIndex)->Begin();
	}
	void VulkanGraphics::End()
	{
		VulkanContext::Get()->GetFrameBuffer(m_SwapChainImageIndex)->End();
		RenderCommand::EndCommand();
		VkSwapchainKHR swapChains[] = { VulkanContext::Get()->GetSwapChain() };

		VkSemaphore waitSemaphores[] = { VulkanContext::Get()->GetRenderFinishedSemaphores(0) };

		VkPresentInfoKHR			presentInfo{};
		presentInfo.sType			= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pImageIndices	= &m_SwapChainImageIndex;
		presentInfo.swapchainCount	= 1;
		presentInfo.pSwapchains		= swapChains;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = waitSemaphores;
		VK_CHECK_RESULT(vkQueuePresentKHR(VulkanContext::Get()->GetPresentQueue(), &presentInfo));
	}
}