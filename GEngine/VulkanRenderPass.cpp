#include "GEpch.h"
#include "VulkanRenderPass.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace GEngine
{
	void VulkanRenderPass::CreateRenderPass(const VulkanRenderPassSpecification& spec)
	{
		VkAttachmentDescription			colorAttachment{};
		colorAttachment.format			= spec.SwapChainImageFormat;
		colorAttachment.samples			= VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;

		VkAttachmentReference			 colorAttachmentRef{};
		colorAttachmentRef.attachment	= 0;

		VkSubpassDescription			 subpass{};
		subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount	= 1;
		subpass.pColorAttachments		= &colorAttachmentRef;

		VkRenderPassCreateInfo          renderPassInfo{};
		renderPassInfo.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount	= 1;
		renderPassInfo.pAttachments		= &colorAttachment;
		renderPassInfo.subpassCount		= 1;
		renderPassInfo.pSubpasses		= &subpass;

		if (vkCreateRenderPass(VulkanContext::GetDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
		{
			GE_CORE_ERROR("failed to create render pass!");
		}
	}
}