#include "GEpch.h"
#include "VulkanRenderPass.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace GEngine
{
	namespace Utils
	{
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
		{
			for (VkFormat format : candidates) {
				VkFormatProperties props;
				vkGetPhysicalDeviceFormatProperties(VulkanContext::Get()->GetPhysicalDevice(), format, &props);

				if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
					return format;
				}
				else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
					return format;
				}
			}

			GE_CORE_CRITICAL("failed to find supported format!");
		}

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
				Attachment.format			= FindSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
				Attachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_CLEAR;
				Attachment.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_STORE;
				Attachment.finalLayout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				break;
			case GEngine::FrameBufferTextureFormat::DEPTH:
				Attachment.format		=  FindSupportedFormat({ VK_FORMAT_D32_SFLOAT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
				Attachment.finalLayout	= VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
				Attachment.finalLayout	= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				break;
			default:
				break;
			}

			return Attachment;
		}

		VkAttachmentReference CreateAttachmentReference(FrameBufferTextureFormat format, int index)
		{
			VkAttachmentReference		Ref{};
			Ref.attachment				= index;
			switch (format)
			{
			case GEngine::FrameBufferTextureFormat::None:
				GE_CORE_ASSERT(false);
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
				Ref.layout				= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				break;
			default:
				break;
			}

			return Ref;
		}
	}


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
			Attachment.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
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
			Attachment.samples			= VK_SAMPLE_COUNT_1_BIT;
			Attachment.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
			Attachment.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
			Attachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_CLEAR;
			Attachment.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_STORE;
			Attachment.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
			Attachment.format			= Utils::FindSupportedFormat({ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
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


	Ref<VulkanRenderPass> VulkanRenderPass::Create(const RenderPassSpecificationForVulkan& spec)
	{
		return CreateRef<VulkanRenderPass>(spec);
	}

	void VulkanRenderPass::CreateRenderPass(const RenderPassSpecification& spec)
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
			VkAttachmentReference		ref = Utils::CreateAttachmentReference(spec.ColorAttachments.at(i).TextureFormat, i);
			colorAttachmentRefs.push_back(ref);
		}

		VkAttachmentReference			depthAttachmentRef{};


		VkSubpassDescription			 subpass{};
		subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount	= static_cast<uint32_t>(colorAttachmentRefs.size());
		subpass.pColorAttachments		= colorAttachmentRefs.data();
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