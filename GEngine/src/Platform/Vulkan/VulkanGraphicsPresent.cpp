#include "GEpch.h"
#include "VulkanGraphicsPresent.h"
#include "GEngine/Application.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "GEngine/Renderer/RenderCommand.h"

namespace GEngine
{
	bool VulkanGraphicsPresent::AquireImage()
	{
		vkWaitForFences(VulkanContext::Get()->GetDevice(), 1, &VulkanContext::Get()->GetCurrentFence(), VK_TRUE, std::numeric_limits<uint64_t>::max());

		VkResult result = vkAcquireNextImageKHR(VulkanContext::Get()->GetDevice(),
			VulkanContext::Get()->GetSwapChain(),
			std::numeric_limits<uint64_t>::max(),
			VulkanContext::Get()->GetCurrentSemaphore(),
			VK_NULL_HANDLE,
			&m_SwapChainImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR ||
			(m_WindowSize.x != 0 && m_WindowSize.y != 0))
		{
			VulkanContext::Get()->RecreateSwapChain(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
			m_WindowSize.x = 0;
			m_WindowSize.y = 0;
			return false;
		}
		return true;
	}
	void VulkanGraphicsPresent::Begin()
	{
		vkResetFences(VulkanContext::Get()->GetDevice(), 1, &VulkanContext::Get()->GetCurrentFence());
		VkCommandBufferBeginInfo    beginInfo{};
		beginInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags				= 0; // Optional
		beginInfo.pInheritanceInfo	= nullptr; // Optional
		VulkanContext::Get()->BeginDrawCommandBuffer();
		vkResetCommandBuffer(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), 0);
		VK_CHECK_RESULT(vkBeginCommandBuffer(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), &beginInfo));
		VulkanContext::Get()->GetFrameBuffer(m_SwapChainImageIndex)->Begin();
	}
	void VulkanGraphicsPresent::End()
	{
		VulkanContext::Get()->GetFrameBuffer(m_SwapChainImageIndex)->End();
		VkCommandBuffer commandBuffer = VulkanContext::Get()->EndDrawCommandBuffer();
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
	}
}