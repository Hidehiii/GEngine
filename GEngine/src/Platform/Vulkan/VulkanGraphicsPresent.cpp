#include "GEpch.h"
#include "Platform/Vulkan/VulkanGraphicsPresent.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanGraphicsAPI.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Graphics/GraphicsResource.h"

namespace
{
	class VulkanPresentationResource final : public GEngine::GraphicsResource
	{
	public:
		explicit VulkanPresentationResource(VkImage image)
			: m_Image(image)
		{
			GE_CORE_ASSERT(m_Image != VK_NULL_HANDLE, "Vulkan presentation image is invalid.");
		}

		GEngine::GraphicsResourceType GetResourceType() const override { return GEngine::GraphicsResourceType::Texture; }
		bool RequiresExplicitStateTransition() const override { return false; }

	private:
		void* GetNativeResource() const override { return reinterpret_cast<void*>(m_Image); }
		VkImage m_Image = VK_NULL_HANDLE;
	};
}

namespace GEngine
{

	VulkanGraphicsPresent::VulkanGraphicsPresent()
	{
		m_Fences.resize(Graphics::GetFrameCount());
		m_CommandBuffers.resize(Graphics::GetFrameCount());
		for (int i = 0; i < m_Fences.size(); i++)
		{
			VkFenceCreateInfo       fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			VK_CHECK_RESULT(vkCreateFence(VulkanContext::Get()->GetDevice(), &fenceInfo, nullptr, &m_Fences[i]));
		}

		std::vector<VkCommandBuffer>	cmds;
		cmds.resize(Graphics::GetFrameCount());

		VkCommandBufferAllocateInfo		allocInfo{};
		allocInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool			= VulkanContext::Get()->GetGraphicsCommandPool();
		allocInfo.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount	= Graphics::GetFrameCount();
		VK_CHECK_RESULT(vkAllocateCommandBuffers(VulkanContext::Get()->GetDevice(), &allocInfo, cmds.data()));

		for (int i = 0; i < m_CommandBuffers.size(); i++)
		{
			m_CommandBuffers.at(i) = VulkanCommandBuffer::Create(cmds.at(i), COMMAND_BUFFER_TYPE_GRAPHICS);
		}
	}

	VulkanGraphicsPresent::~VulkanGraphicsPresent()
	{
		// m_CommandBuffers is owned by the base presenter and is destroyed next.
		// Finish its work before releasing the fences and command-buffer wrappers.
		const VkDevice device = VulkanContext::Get()->GetDevice();
		if (device != VK_NULL_HANDLE)
		{
			VulkanContext::Get()->WaitForIdle();
			for (const auto fence : m_Fences)
				vkDestroyFence(device, fence, nullptr);
		}
		m_Fences.clear();
	}
	bool VulkanGraphicsPresent::AquireImage()
	{
		const VkExtent2D swapChainExtent = VulkanContext::Get()->GetSwapChainExtent();
		if (swapChainExtent.width != Graphics::GetViewportWidth() ||
			swapChainExtent.height != Graphics::GetViewportHeight())
		{
			VulkanContext::Get()->RecreateSwapChain(Graphics::GetViewportWidth(), Graphics::GetViewportHeight());
			GE_CORE_INFO("Recreate swapChain");
			return false;
		}

		VK_CHECK_RESULT(vkWaitForFences(VulkanContext::Get()->GetDevice(), 1, &m_Fences.at(Graphics::GetFrame()), VK_TRUE, std::numeric_limits<uint64_t>::max()));
		VulkanContext::Get()->CollectDeferredReleases();
		m_AcquireSemaphore = VulkanContext::Get()->GetSemaphore();
		VkResult result = vkAcquireNextImageKHR(VulkanContext::Get()->GetDevice(),
			VulkanContext::Get()->GetSwapChain(),
			std::numeric_limits<uint64_t>::max(),
			m_AcquireSemaphore,
			VK_NULL_HANDLE,
			&m_SwapChainImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			m_AcquireSemaphore = VK_NULL_HANDLE;
			VulkanContext::Get()->RecreateSwapChain(Graphics::GetViewportWidth(), Graphics::GetViewportHeight());
			GE_CORE_INFO("Recreate swapChain");
			return false;
		}
		VK_CHECK_RESULT(result);

		return true;
	}

	bool VulkanGraphicsPresent::AcquireFrame(FrameContext& frameContext)
	{
		if (!AquireImage())
			return false;

		frameContext.MarkAcquired(m_SwapChainImageIndex);
		return true;
	}
	Ref<GraphicsResource> VulkanGraphicsPresent::GetPresentationResource() const
	{
		const auto images = VulkanContext::Get()->GetSwapChainImage();
		GE_CORE_ASSERT(m_SwapChainImageIndex < images.size(), "Vulkan presentation image index is invalid.");
		// The legacy Vulkan render pass owns this image's layout transitions.  The
		// graph still tracks the resource and ordering, but must not duplicate the
		// render pass's Present <-> ColorAttachment layout work.
		return CreateRef<VulkanPresentationResource>(images.at(m_SwapChainImageIndex));
	}
	void VulkanGraphicsPresent::Begin()
	{
		vkResetFences(VulkanContext::Get()->GetDevice(), 1, &m_Fences.at(Graphics::GetFrame()));
		// BeginPresentRender owns reset + begin for a presentation command buffer.
		// Beginning it here first would attempt to reset an already-recording buffer.
		std::dynamic_pointer_cast<VulkanCommandBuffer>(m_CommandBuffers.at(Graphics::GetFrame()))->BeginPresentRender(std::static_pointer_cast<FrameBuffer>(VulkanContext::Get()->GetFrameBuffer(m_SwapChainImageIndex)));
	}
	void VulkanGraphicsPresent::End()
	{
		std::dynamic_pointer_cast<VulkanCommandBuffer>(m_CommandBuffers.at(Graphics::GetFrame()))->EndPresentRender();
	}

	uint64_t VulkanGraphicsPresent::Submit()
	{
		VkSemaphore presentSemaphore = VulkanContext::Get()->GetSemaphore();
		auto& graphicsAPI = dynamic_cast<VulkanGraphicsAPI&>(Graphics::GetRenderDevice());
		graphicsAPI.SubmitPresentationCommandBuffer(m_CommandBuffers.at(Graphics::GetFrame()), m_AcquireSemaphore,
			presentSemaphore, m_Fences.at(Graphics::GetFrame()));
		VkSwapchainKHR swapChains[] = { VulkanContext::Get()->GetSwapChain() };

		VkSemaphore presentWaitSemaphores[] = { presentSemaphore };

		VkPresentInfoKHR			presentInfo{};
		presentInfo.sType			= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pImageIndices	= &m_SwapChainImageIndex;
		presentInfo.swapchainCount	= 1;
		presentInfo.pSwapchains		= swapChains;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = presentWaitSemaphores;
		VK_CHECK_RESULT(vkQueuePresentKHR(VulkanContext::Get()->GetPresentQueue(), &presentInfo));

		m_AcquireSemaphore = VK_NULL_HANDLE;
		return 0;
	}

}
