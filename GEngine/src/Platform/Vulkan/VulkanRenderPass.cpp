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
		std::vector<VkAttachmentDescription2>  attachments;
		for (int i = 0; i < spec.ColorAttachmentsFormat.size(); i++)
		{
			VkAttachmentDescription2	Attachment{};
			Attachment.sType			= VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
			Attachment.samples			= VK_SAMPLE_COUNT_1_BIT;
			Attachment.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
			Attachment.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
			Attachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			Attachment.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			Attachment.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
			Attachment.format			= spec.ColorAttachmentsFormat.at(i);
			Attachment.finalLayout		= spec.ColorAttachmentsFinalLayout.at(i);
			attachments.push_back(Attachment);
			if (spec.Samples > 1)
			{
				Attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				Attachment.samples = Utils::SampleCountToVulkanFlag(spec.Samples);
				attachments.push_back(Attachment);
			}
		}
		if (spec.EnableDepthStencilAttachment)
		{
			VkAttachmentDescription2	Attachment{};
			Attachment.sType			= VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
			Attachment.samples			= VK_SAMPLE_COUNT_1_BIT;
			Attachment.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
			Attachment.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
			Attachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_CLEAR;
			Attachment.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_STORE;
			Attachment.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
			Attachment.format			= Utils::FindSupportedFormat({ VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
			Attachment.finalLayout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			attachments.push_back(Attachment);
			if (spec.Samples > 1)
			{
				Attachment.storeOp			= VK_ATTACHMENT_STORE_OP_DONT_CARE;
				Attachment.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
				Attachment.samples = Utils::SampleCountToVulkanFlag(spec.Samples);
				attachments.push_back(Attachment);
			}
		}

		std::vector<VkAttachmentReference2> colorAttachmentRefs;
		std::vector<VkAttachmentReference2> resolveAttachmentRefs;
		if (spec.Samples > 1)
		{
			int index = 0;
			for (int i = 0; i < spec.ColorAttachmentsFormat.size(); i++)
			{
				VkAttachmentReference2		Ref{};
				Ref.sType					= VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
				Ref.attachment				= index;
				Ref.layout					= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				Ref.aspectMask				= VK_IMAGE_ASPECT_COLOR_BIT;
				resolveAttachmentRefs.push_back(Ref);
				index++;
				Ref.attachment				= index;
				colorAttachmentRefs.push_back(Ref);
				index++;
			}
		}
		else
		{
			for (int i = 0; i < spec.ColorAttachmentsFormat.size(); i++)
			{
				VkAttachmentReference2		Ref{};
				Ref.sType					= VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
				Ref.attachment				= i;
				Ref.layout					= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				Ref.aspectMask				= VK_IMAGE_ASPECT_COLOR_BIT;
				colorAttachmentRefs.push_back(Ref);
			}
		}
		



		VkSubpassDescription2			 subpass{};
		subpass.sType					= VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
		subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount	= static_cast<uint32_t>(colorAttachmentRefs.size());
		subpass.pColorAttachments		= colorAttachmentRefs.data();
		subpass.pResolveAttachments		= resolveAttachmentRefs.data();
		subpass.pDepthStencilAttachment = nullptr;


		VkAttachmentReference2			depthAttachmentRef{};
		depthAttachmentRef.sType		= VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
		if (spec.EnableDepthStencilAttachment)
		{
			depthAttachmentRef.attachment				= colorAttachmentRefs.size();
			depthAttachmentRef.layout					= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			if (spec.Samples > 1)
			{
				depthAttachmentRef.attachment			= spec.ColorAttachmentsFormat.size() * 2 + 1;

				VkSubpassDescriptionDepthStencilResolve	subpassDepthStencilResolve{};

				VkAttachmentReference2					ref{};
				ref										= Utils::CreateAttachmentReference2(FrameBufferTextureFormat::DEPTH24STENCIL8, spec.ColorAttachmentsFormat.size() * 2);

				subpassDepthStencilResolve.sType							= VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE;
				subpassDepthStencilResolve.pDepthStencilResolveAttachment	= &ref;
				subpassDepthStencilResolve.depthResolveMode					= VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
				subpassDepthStencilResolve.stencilResolveMode				= VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
				subpass.pNext												= &subpassDepthStencilResolve;
			}
			else
			{
				depthAttachmentRef = Utils::CreateAttachmentReference2(FrameBufferTextureFormat::DEPTH24STENCIL8, spec.ColorAttachmentsFormat.size());
			}
			subpass.pDepthStencilAttachment = &depthAttachmentRef;
		}
		


		VkSubpassDependency2			 dependency{};
		dependency.sType				= VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
		dependency.srcSubpass			= VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass			= 0;
		dependency.srcStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask		= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency.dstStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		

		VkRenderPassCreateInfo2          renderPassInfo{};
		renderPassInfo.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
		renderPassInfo.attachmentCount	= static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments		= attachments.data();
		renderPassInfo.subpassCount		= 1;
		renderPassInfo.pSubpasses		= &subpass;
		renderPassInfo.dependencyCount	= 1;
		renderPassInfo.pDependencies	= &dependency;

		VK_CHECK_RESULT(vkCreateRenderPass2(VulkanContext::Get()->GetDevice(), &renderPassInfo, nullptr, &m_RenderPass));
	}

	VulkanRenderPass::~VulkanRenderPass()
	{
		if (VulkanContext::Get()->GetDevice())
		{
			vkDestroyRenderPass(VulkanContext::Get()->GetDevice(), m_RenderPass, nullptr);
		}
		
	}


	Ref<VulkanRenderPass> VulkanRenderPass::Create(const RenderPassSpecificationForVulkan& spec)
	{
		return CreateRef<VulkanRenderPass>(spec);
	}

	void VulkanRenderPass::CreateRenderPass(const RenderPassSpecification& spec)
	{
		std::vector<VkAttachmentDescription2>  attachments;
		for (int i = 0; i < spec.ColorAttachments.size(); i++)
		{
			VkAttachmentDescription2		des = Utils::CreateAttachmentDescription2(spec.ColorAttachments.at(i).TextureFormat, VK_SAMPLE_COUNT_1_BIT);
			attachments.push_back(des);
			if (spec.Samples > 1)
			{
				des = Utils::CreateAttachmentDescription2(spec.ColorAttachments.at(i).TextureFormat, Utils::SampleCountToVulkanFlag(spec.Samples));
				attachments.push_back(des);
			}
		}
		if (spec.DepthAttachment.TextureFormat != FrameBufferTextureFormat::None)
		{
			VkAttachmentDescription2		des = Utils::CreateAttachmentDescription2(spec.DepthAttachment.TextureFormat, VK_SAMPLE_COUNT_1_BIT);
			attachments.push_back(des);
			if (spec.Samples > 1)
			{
				des = Utils::CreateAttachmentDescription2(spec.DepthAttachment.TextureFormat, Utils::SampleCountToVulkanFlag(spec.Samples));
				attachments.push_back(des);
			}
		}

		std::vector<VkAttachmentReference2> colorAttachmentRefs;
		std::vector<VkAttachmentReference2> resolveAttachmentRefs;
		if (spec.Samples > 1)
		{
			int index = 0;
			for (int i = 0; i < spec.ColorAttachments.size(); i++)
			{
				VkAttachmentReference2		ref = Utils::CreateAttachmentReference2(spec.ColorAttachments.at(i).TextureFormat, index);
				resolveAttachmentRefs.push_back(ref);
				index++;
				ref = Utils::CreateAttachmentReference2(spec.ColorAttachments.at(i).TextureFormat, index);
				colorAttachmentRefs.push_back(ref);
				index++;
			}
		}
		else
		{
			for (int i = 0; i < spec.ColorAttachments.size(); i++)
			{
				VkAttachmentReference2		ref = Utils::CreateAttachmentReference2(spec.ColorAttachments.at(i).TextureFormat, i);
				colorAttachmentRefs.push_back(ref);
			}
		}
		

		VkAttachmentReference2			depthAttachmentRef{};
		depthAttachmentRef.sType		= VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;

		VkSubpassDescription2			 subpass{};
		subpass.sType					= VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
		subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount	= static_cast<uint32_t>(colorAttachmentRefs.size());
		subpass.pColorAttachments		= colorAttachmentRefs.data();
		subpass.pResolveAttachments		= resolveAttachmentRefs.data();
		subpass.pDepthStencilAttachment = nullptr;
		if (spec.DepthAttachment.TextureFormat != FrameBufferTextureFormat::None)
		{
			if (spec.Samples > 1)
			{
				depthAttachmentRef = Utils::CreateAttachmentReference2(spec.DepthAttachment.TextureFormat, spec.ColorAttachments.size() * 2 + 1);

				VkSubpassDescriptionDepthStencilResolve	subpassDepthStencilResolve{};

				VkAttachmentReference2	ref{};
				ref = Utils::CreateAttachmentReference2(spec.DepthAttachment.TextureFormat, spec.ColorAttachments.size() * 2);

				subpassDepthStencilResolve.sType							= VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE;
				subpassDepthStencilResolve.pDepthStencilResolveAttachment	= &ref;
				subpassDepthStencilResolve.depthResolveMode					= VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
				subpassDepthStencilResolve.stencilResolveMode				= VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
				subpass.pNext												= &subpassDepthStencilResolve;
			}
			else
			{
				depthAttachmentRef = Utils::CreateAttachmentReference2(spec.DepthAttachment.TextureFormat, spec.ColorAttachments.size());
			}
			subpass.pDepthStencilAttachment = &depthAttachmentRef;
		}

		

		VkSubpassDependency2			 dependency{};
		dependency.sType				= VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
		dependency.srcSubpass			= VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass			= 0;
		dependency.srcStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask		= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency.dstStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo2          renderPassInfo{};
		renderPassInfo.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
		renderPassInfo.attachmentCount	= static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments		= attachments.data();
		renderPassInfo.subpassCount		= 1;
		renderPassInfo.pSubpasses		= &subpass;
		renderPassInfo.dependencyCount	= 1;
		renderPassInfo.pDependencies	= &dependency;

		VK_CHECK_RESULT(vkCreateRenderPass2(VulkanContext::Get()->GetDevice(), &renderPassInfo, nullptr, &m_RenderPass));
	}
	
}