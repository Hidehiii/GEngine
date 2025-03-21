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
		m_Fences.resize(Renderer::GetFramesInFlight());
		s_CommandBuffers.resize(Renderer::GetFramesInFlight());
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
			s_CommandBuffers.at(i) = VulkanCommandBuffer::Create(cmds.at(i), CommandBufferType::Graphics);
		}
	}
	bool VulkanGraphicsPresent::AquireImage()
	{
		VK_CHECK_RESULT(vkWaitForFences(VulkanContext::Get()->GetDevice(), 1, &m_Fences.at(Renderer::GetCurrentFrame()), VK_TRUE, std::numeric_limits<uint64_t>::max()));
		VkSemaphore s = VulkanContext::Get()->GetSemaphore();
		VkResult result = vkAcquireNextImageKHR(VulkanContext::Get()->GetDevice(),
			VulkanContext::Get()->GetSwapChain(),
			std::numeric_limits<uint64_t>::max(),
			s,
			VK_NULL_HANDLE,
			&m_SwapChainImageIndex);
		VkSemaphore submitWaitSemaphores[] = { s };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSubmitInfo					submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 0;
		submitInfo.pCommandBuffers = nullptr;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = submitWaitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = nullptr;
		VK_CHECK_RESULT(vkQueueSubmit(VulkanContext::Get()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
		if (result == VK_ERROR_OUT_OF_DATE_KHR ||
			(m_WindowResize.x != 0 && m_WindowResize.y != 0))
		{
			VulkanContext::Get()->RecreateSwapChain(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
			m_WindowResize.x = 0;
			m_WindowResize.y = 0;
			
			GE_CORE_INFO("Recreate swapChain");
			return false;
		}

		return true;
	}
	void VulkanGraphicsPresent::Begin()
	{
		vkResetFences(VulkanContext::Get()->GetDevice(), 1, &m_Fences.at(Renderer::GetCurrentFrame()));
		VkCommandBufferBeginInfo    beginInfo{};
		beginInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags				= 0; // Optional
		beginInfo.pInheritanceInfo	= nullptr; // Optional
		vkResetCommandBuffer(std::dynamic_pointer_cast<VulkanCommandBuffer>(s_CommandBuffers.at(Renderer::GetCurrentFrame()))->GetCommandBuffer(), 0);
		VK_CHECK_RESULT(vkBeginCommandBuffer(std::dynamic_pointer_cast<VulkanCommandBuffer>(s_CommandBuffers.at(Renderer::GetCurrentFrame()))->GetCommandBuffer(), &beginInfo));
		s_CommandBuffers.at(Renderer::GetCurrentFrame())->Begin(std::static_pointer_cast<FrameBuffer>(VulkanContext::Get()->GetFrameBuffer(m_SwapChainImageIndex)));
	}
	void VulkanGraphicsPresent::End()
	{
		VulkanContext::Get()->GetFrameBuffer(m_SwapChainImageIndex)->End(s_CommandBuffers.at(Renderer::GetCurrentFrame()).get());

		Ref<VulkanCommandBuffer> cmd = std::dynamic_pointer_cast<VulkanCommandBuffer>(s_CommandBuffers.at(Renderer::GetCurrentFrame()));

		VkCommandBuffer commandBuffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(s_CommandBuffers.at(Renderer::GetCurrentFrame()))->GetCommandBuffer();
		VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
		std::vector<VkSemaphore> submitWaitSemaphores = cmd->GetWaitSemaphores();
		std::vector<VkPipelineStageFlags> waitStages(submitWaitSemaphores.size(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		VkSemaphore s = VulkanContext::Get()->GetSemaphore();
		VkSemaphore signalSemaphores[] = { s };

		VkSubmitInfo					submitInfo{};
		submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount	= 1;
		submitInfo.pCommandBuffers		= &commandBuffer;
		submitInfo.waitSemaphoreCount	= 1;
		submitInfo.pWaitSemaphores		= submitWaitSemaphores.data();
		submitInfo.pWaitDstStageMask	= waitStages.data();
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores	= signalSemaphores;

		VK_CHECK_RESULT(vkQueueSubmit(VulkanContext::Get()->GetGraphicsQueue(), 1, &submitInfo, m_Fences.at(Renderer::GetCurrentFrame())));
		VkSwapchainKHR swapChains[] = { VulkanContext::Get()->GetSwapChain() };

		VkSemaphore presentWaitSemaphores[] = { s };

		VkPresentInfoKHR			presentInfo{};
		presentInfo.sType			= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pImageIndices	= &m_SwapChainImageIndex;
		presentInfo.swapchainCount	= 1;
		presentInfo.pSwapchains		= swapChains;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = presentWaitSemaphores;
		VK_CHECK_RESULT(vkQueuePresentKHR(VulkanContext::Get()->GetPresentQueue(), &presentInfo));

		cmd->ClearSignalSemaphores();
		cmd->ClearWaitSemaphores();
	}

}