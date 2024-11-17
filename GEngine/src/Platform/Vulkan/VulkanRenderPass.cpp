#include "GEpch.h"
#include "VulkanRenderPass.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace GEngine
{


	VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& spec)
	{
		CreateRenderPass(spec);
	}

	VulkanRenderPass::VulkanRenderPass(const RenderPassSpecificationForVulkan& spec)
	{
		std::vector<VkAttachmentDescription>  attachments;
		for (int i = 0; i < spec.ColorAttachmentsFormat.size(); i++)
		{
			VkAttachmentDescription		Attachment{};
			Attachment.samples			= VK_SAMPLE_COUNT_1_BIT;
			Attachment.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
			Attachment.storeOp			= spec.Samples == 1 ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
			Attachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			Attachment.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			Attachment.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
			Attachment.format			= spec.ColorAttachmentsFormat.at(i);
			Attachment.finalLayout		= spec.ColorAttachmentsFinalLayout.at(i);
			attachments.push_back(Attachment);
		}
		if (spec.EnableDepthStencilAttachment)
		{
			VkAttachmentDescription		Attachment{};
			Attachment.samples			= Utils::SampleCountToVulkanFlag(spec.Samples);
			Attachment.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
			Attachment.storeOp			= spec.Samples == 1 ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
			Attachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_CLEAR;
			Attachment.stencilStoreOp	= spec.Samples == 1 ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
			Attachment.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
			Attachment.format			= Utils::FindSupportedFormat({ VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
			Attachment.finalLayout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			attachments.push_back(Attachment);
		}

		std::vector<VkAttachmentReference> colorAttachmentRefs;
		for (int i = 0; i < spec.ColorAttachmentsFormat.size(); i++)
		{
			VkAttachmentReference		Ref{};
			Ref.attachment				= i;
			Ref.layout					= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachmentRefs.push_back(Ref);
		}
		VkAttachmentReference			depthAttachmentRef{};
		if (spec.EnableDepthStencilAttachment)
		{
			VkAttachmentReference		Ref{};
			depthAttachmentRef.attachment				= colorAttachmentRefs.size();
			depthAttachmentRef.layout					= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		VkSubpassDescription			 subpass{};
		subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount	= static_cast<uint32_t>(colorAttachmentRefs.size());
		subpass.pColorAttachments		= colorAttachmentRefs.data();
		subpass.pDepthStencilAttachment = nullptr;
		if (spec.EnableDepthStencilAttachment)
		{
			subpass.pDepthStencilAttachment = &depthAttachmentRef;
		}


		VkSubpassDependency			 dependency{};
		dependency.srcSubpass		= VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass		= 0;
		dependency.srcStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask	= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency.dstStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask	= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo          renderPassInfo{};
		renderPassInfo.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount	= static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments		= attachments.data();
		renderPassInfo.subpassCount		= 1;
		renderPassInfo.pSubpasses		= &subpass;
		renderPassInfo.dependencyCount	= 1;
		renderPassInfo.pDependencies	= &dependency;

		VK_CHECK_RESULT(vkCreateRenderPass(VulkanContext::Get()->GetDevice(), &renderPassInfo, nullptr, &m_RenderPass));
	}

	VulkanRenderPass::~VulkanRenderPass()
	{
		vkDestroyRenderPass(VulkanContext::Get()->GetDevice(), m_RenderPass, nullptr);
	}


	Ref<VulkanRenderPass> VulkanRenderPass::Create(const RenderPassSpecificationForVulkan& spec)
	{
		return CreateRef<VulkanRenderPass>(spec);
	}

	void VulkanRenderPass::CreateRenderPass(const RenderPassSpecification& spec)
	{
		std::vector<VkAttachmentDescription>  attachments;
		for (int i = 0; i < spec.ColorAttachments.size(); i++)
		{
			VkAttachmentDescription		des = Utils::CreateAttachmentDescription(spec.ColorAttachments.at(i).TextureFormat, VK_SAMPLE_COUNT_1_BIT);
			attachments.push_back(des);
			if (spec.Samples > 1)
			{
				des = Utils::CreateAttachmentDescription(spec.ColorAttachments.at(i).TextureFormat, Utils::SampleCountToVulkanFlag(spec.Samples));
				attachments.push_back(des);
			}
		}
		if (spec.DepthAttachment.TextureFormat != FrameBufferTextureFormat::None)
		{
			VkAttachmentDescription		des = Utils::CreateAttachmentDescription(spec.DepthAttachment.TextureFormat, VK_SAMPLE_COUNT_1_BIT);
			attachments.push_back(des);
			if (spec.Samples > 1)
			{
				des = Utils::CreateAttachmentDescription(spec.DepthAttachment.TextureFormat, Utils::SampleCountToVulkanFlag(spec.Samples));
				attachments.push_back(des);
			}
		}

		std::vector<VkAttachmentReference> colorAttachmentRefs;
		std::vector<VkAttachmentReference> resolveAttachmentRefs;
		if (spec.Samples > 1)
		{
			int index = 0;
			for (int i = 0; i < spec.ColorAttachments.size(); i++)
			{
				VkAttachmentReference		ref = Utils::CreateAttachmentReference(spec.ColorAttachments.at(i).TextureFormat, index);
				colorAttachmentRefs.push_back(ref);
				index++;
				ref = Utils::CreateAttachmentReference(spec.ColorAttachments.at(i).TextureFormat, index);
				resolveAttachmentRefs.push_back(ref);
				index++;
			}
		}
		else
		{
			for (int i = 0; i < spec.ColorAttachments.size(); i++)
			{
				VkAttachmentReference		ref = Utils::CreateAttachmentReference(spec.ColorAttachments.at(i).TextureFormat, i);
				colorAttachmentRefs.push_back(ref);
			}
		}
		

		VkAttachmentReference			depthAttachmentRef{};


		VkSubpassDescription			 subpass{};
		subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount	= static_cast<uint32_t>(colorAttachmentRefs.size());
		subpass.pColorAttachments		= colorAttachmentRefs.data();
		subpass.pResolveAttachments		= resolveAttachmentRefs.data();
		subpass.pDepthStencilAttachment = nullptr;
		if (spec.DepthAttachment.TextureFormat != FrameBufferTextureFormat::None)
		{
			depthAttachmentRef			= Utils::CreateAttachmentReference(spec.DepthAttachment.TextureFormat, spec.ColorAttachments.size());
			subpass.pDepthStencilAttachment = &depthAttachmentRef;
		}
		

		VkSubpassDependency			 dependency{};
		dependency.srcSubpass			= VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass			= 0;
		dependency.srcStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask		= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency.dstStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo          renderPassInfo{};
		renderPassInfo.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount	= static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments		= attachments.data();
		renderPassInfo.subpassCount		= 1;
		renderPassInfo.pSubpasses		= &subpass;
		renderPassInfo.dependencyCount	= 1;
		renderPassInfo.pDependencies	= &dependency;

		VK_CHECK_RESULT(vkCreateRenderPass(VulkanContext::Get()->GetDevice(), &renderPassInfo, nullptr, &m_RenderPass));
	}
	
}