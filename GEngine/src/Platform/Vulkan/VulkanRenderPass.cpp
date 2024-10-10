#include "GEpch.h"
#include "VulkanRenderPass.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace GEngine
{
	namespace Utils
	{
		VkAttachmentDescription CreateAttachmentDescription(FrameBufferTextureFormat format, int samples = 1)
		{
			VkAttachmentDescription		Attachment{};
			Attachment.samples			= VK_SAMPLE_COUNT_1_BIT;
			Attachment.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
			Attachment.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
			Attachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			Attachment.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			Attachment.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
			Attachment.finalLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			switch (format)
			{
			case GEngine::FrameBufferTextureFormat::None:
				GE_CORE_ASSERT(false, "None is not a valid FrameBufferTextureFormat!");
				break;
			case GEngine::FrameBufferTextureFormat::RGBA8:
				Attachment.format		= VK_FORMAT_R8G8B8A8_SRGB;
				break;
			case GEngine::FrameBufferTextureFormat::RED_INTEGER:
				Attachment.format		= VK_FORMAT_R8_SINT;
				break;
			case GEngine::FrameBufferTextureFormat::DEPTH24STENCIL8:
				Attachment.format			= VK_FORMAT_D24_UNORM_S8_UINT;
				Attachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_CLEAR;
				Attachment.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_STORE;
				Attachment.finalLayout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				break;
			case GEngine::FrameBufferTextureFormat::DEPTH:
				Attachment.format		= VK_FORMAT_D32_SFLOAT;
				Attachment.finalLayout	= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				break;
			default:
				break;
			}
			return Attachment;
		}
	}


	void VulkanRenderPass::CreateRenderPass(const VulkanRenderPassSpecification& spec)
	{
		std::vector<VkAttachmentDescription>	attachments;
		VkAttachmentDescription			colorAttachment{};
		VkAttachmentDescription			depthAttachment{};
		colorAttachment					= Utils::CreateAttachmentDescription(spec.ColorAttachmentFormats[0]);
		depthAttachment					= Utils::CreateAttachmentDescription(spec.DepthAttachmentFormat);

		VkAttachmentReference			 colorAttachmentRef{};
		colorAttachmentRef.attachment	= 0;

		VkAttachmentReference			depthAttachmentRef{};
		depthAttachmentRef.attachment	= 1;
		depthAttachmentRef.layout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription			 subpass{};
		subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount	= 1;
		subpass.pColorAttachments		= &colorAttachmentRef;
		subpass.pDepthStencilAttachment	= &depthAttachmentRef;

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