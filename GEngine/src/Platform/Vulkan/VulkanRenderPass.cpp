#include "GEpch.h"
#include "VulkanRenderPass.h"
#include "Platform/Vulkan/VulkanUtils.h"
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
			switch (format)
			{
			case GEngine::FrameBufferTextureFormat::None:
				GE_CORE_ASSERT(false, "None is not a valid FrameBufferTextureFormat!");
				break;
			case GEngine::FrameBufferTextureFormat::RGBA8:
				Attachment.format		= VK_FORMAT_R8G8B8A8_UNORM;
				Attachment.finalLayout	= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				break;
			case GEngine::FrameBufferTextureFormat::RED_INTEGER:
				Attachment.format		= VK_FORMAT_R8_SINT;
				Attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				break;
			case GEngine::FrameBufferTextureFormat::DEPTH24STENCIL8:
				Attachment.format			= VK_FORMAT_D24_UNORM_S8_UINT;
				Attachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_CLEAR;
				Attachment.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_STORE;
				Attachment.finalLayout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				break;
			case GEngine::FrameBufferTextureFormat::DEPTH:
				Attachment.format		= VK_FORMAT_D32_SFLOAT;
				Attachment.finalLayout	= VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
				break;
			default:
				break;
			}

			return Attachment;
		}

		VkAttachmentReference CreateAttachmentReference(FrameBufferTextureFormat format)
		{
			VkAttachmentReference		Ref{};
			Ref.attachment				= 1;
			switch (format)
			{
			case GEngine::FrameBufferTextureFormat::None:
				GE_CORE_ASSERT(false, "");
				GE_CORE_ERROR("Unknown framebuffer texture format");
				break;
			case GEngine::FrameBufferTextureFormat::RGBA8:
				Ref.layout				= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				break;
			case GEngine::FrameBufferTextureFormat::RED_INTEGER:
				break;
			case GEngine::FrameBufferTextureFormat::DEPTH24STENCIL8:
				Ref.layout				= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				break;
			case GEngine::FrameBufferTextureFormat::DEPTH:
				Ref.layout				= VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
				break;
			default:
				break;
			}

			return Ref;
		}
	}


	VulkanRenderPass::VulkanRenderPass(const VulkanRenderPassSpecification& spec)
	{
		CreateRenderPass(spec);
	}

	void VulkanRenderPass::CreateRenderPass(const VulkanRenderPassSpecification& spec)
	{
		std::vector<VkAttachmentDescription>  attachments;
		for (int i = 0; i < spec.ColorAttachments.size(); i++)
		{
			VkAttachmentDescription		des = Utils::CreateAttachmentDescription(spec.ColorAttachments.at(i).TextureFormat);
			attachments.push_back(des);
		}
		if (spec.DepthAttachment.TextureFormat != FrameBufferTextureFormat::None)
		{
			VkAttachmentDescription		des = Utils::CreateAttachmentDescription(spec.DepthAttachment.TextureFormat);
			attachments.push_back(des);
		}

		std::vector<VkAttachmentReference> colorAttachmentRefs;
		for (int i = 0; i < spec.ColorAttachments.size(); i++)
		{
			VkAttachmentReference		ref = Utils::CreateAttachmentReference(spec.ColorAttachments.at(i).TextureFormat);
			colorAttachmentRefs.push_back(ref);
		}
		if (colorAttachmentRefs.size() == 0)
		{
			VkAttachmentReference		ref{};
			ref.attachment				= 0;
			colorAttachmentRefs.push_back(ref);
		}

		VkAttachmentReference			depthAttachmentRef{};

		VkSubpassDescription			 subpass{};
		subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount	= static_cast<uint32_t>(colorAttachmentRefs.size());
		subpass.pColorAttachments		= colorAttachmentRefs.data();
		if (spec.DepthAttachment.TextureFormat != FrameBufferTextureFormat::None)
		{
			depthAttachmentRef			= Utils::CreateAttachmentReference(spec.DepthAttachment.TextureFormat);
			subpass.pDepthStencilAttachment = &depthAttachmentRef;
		}
		subpass.pDepthStencilAttachment	= &depthAttachmentRef;

		VkSubpassDependency			 dependency{};
		dependency.srcSubpass			= VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass			= 0;
		dependency.srcStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask		= 0;
		dependency.dstStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo          renderPassInfo{};
		renderPassInfo.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount	= static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments		= attachments.data();
		renderPassInfo.subpassCount		= 1;
		renderPassInfo.pSubpasses		= &subpass;
		renderPassInfo.dependencyCount	= 1;
		renderPassInfo.pDependencies	= &dependency;

		VK_CHECK_RESULT(vkCreateRenderPass(VulkanContext::GetDevice(), &renderPassInfo, nullptr, &m_RenderPass));
	}
}