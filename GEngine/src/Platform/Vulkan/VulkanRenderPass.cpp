#include "GEpch.h"
#include "VulkanRenderPass.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace GEngine
{


	VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& spec)
	{
		m_Specification = spec;
		CreateRenderPass();
	}

	VulkanRenderPass::VulkanRenderPass(const RenderPassSpecificationForVulkan& spec)
	{
		// for vulkan swap chain, subpass has only one subpass, 
		// and the color attachment is the swap chain image, so we don't need to create a render pass for it
		m_Specification.Samples		= spec.Samples;

		std::vector<VkAttachmentDescription2>	attachments;
		VkAttachmentDescription2				attachmentDesc{};
		std::vector<VkAttachmentReference2>		colorAttachmentRefs;
		std::vector<VkAttachmentReference2>		multiSampleColorAttachmentRefs;
		VkAttachmentReference2					ref{};
		uint32_t								attachmentIndex = 0;
		VkAttachmentReference2					depthAttachmentRef{};
		VkAttachmentReference2					multiSampleDepthAttachmentRef{};
		VkSubpassDescriptionDepthStencilResolve	subpassDepthStencilResolve{};
		VkSubpassDescription2					subpass{};
		VkSubpassDependency2					dependency{};

		// create attachment desc
		for (int i = 0; i < spec.ColorAttachmentsFormat.size(); i++)
		{
			attachmentDesc.sType			= VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
			attachmentDesc.samples			= VK_SAMPLE_COUNT_1_BIT;
			attachmentDesc.loadOp			= spec.ColorBegin;
			attachmentDesc.storeOp			= spec.ColorEnd;
			attachmentDesc.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDesc.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDesc.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDesc.format			= spec.ColorAttachmentsFormat.at(i);
			attachmentDesc.finalLayout		= spec.ColorAttachmentsFinalLayout.at(i);

			attachments.push_back(attachmentDesc);
			
		}
		if (spec.EnableDepthStencilAttachment)
		{
			attachmentDesc.sType			= VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
			attachmentDesc.samples			= VK_SAMPLE_COUNT_1_BIT;
			attachmentDesc.loadOp			= spec.DepthStencilrBegin;
			attachmentDesc.storeOp			= spec.DepthStencilEnd;
			attachmentDesc.stencilLoadOp	= spec.DepthStencilrBegin;
			attachmentDesc.stencilStoreOp	= spec.DepthStencilEnd;
			attachmentDesc.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDesc.format			= Utils::FindSupportedFormat({ VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
			attachmentDesc.finalLayout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			attachments.push_back(attachmentDesc);
		}
		if (spec.Samples > 1)
		{
			for (int i = 0; i < spec.ColorAttachmentsFormat.size(); i++)
			{
				attachmentDesc.sType			= VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
				attachmentDesc.samples			= Utils::SampleCountToVulkanFlag(spec.Samples);
				attachmentDesc.loadOp			= spec.ColorBegin;
				attachmentDesc.storeOp			= VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachmentDesc.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDesc.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachmentDesc.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
				attachmentDesc.format			= spec.ColorAttachmentsFormat.at(i);
				attachmentDesc.finalLayout		= spec.ColorAttachmentsFinalLayout.at(i);

				attachments.push_back(attachmentDesc);
			}
			if (spec.EnableDepthStencilAttachment)
			{
				attachmentDesc.sType			= VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
				attachmentDesc.samples			= Utils::SampleCountToVulkanFlag(spec.Samples);
				attachmentDesc.loadOp			= spec.DepthStencilrBegin;
				attachmentDesc.storeOp			= VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachmentDesc.stencilLoadOp	= spec.DepthStencilrBegin;
				attachmentDesc.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachmentDesc.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
				attachmentDesc.format			= Utils::FindSupportedFormat({ VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
				attachmentDesc.finalLayout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

				attachments.push_back(attachmentDesc);
			}
		}
		// create attachment reference
		for (int i = 0; i < spec.ColorAttachmentsFormat.size(); i++)
		{
			ref.sType		= VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
			ref.attachment	= attachmentIndex;
			ref.layout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			ref.aspectMask	= VK_IMAGE_ASPECT_COLOR_BIT;

			colorAttachmentRefs.push_back(ref);

			attachmentIndex++;
		}
		if (spec.EnableDepthStencilAttachment)
		{
			depthAttachmentRef.sType		= VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
			depthAttachmentRef.attachment	= attachmentIndex;
			depthAttachmentRef.layout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthAttachmentRef.aspectMask	= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

			attachmentIndex++;
		}
		// create multi sample attachment reference
		if (spec.Samples > 1)
		{
			// color attachment reference
			for (int i = 0; i < spec.ColorAttachmentsFormat.size(); i++)
			{
				ref.sType		= VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
				ref.attachment	= attachmentIndex;
				ref.layout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				ref.aspectMask	= VK_IMAGE_ASPECT_COLOR_BIT;

				multiSampleColorAttachmentRefs.push_back(ref);

				attachmentIndex++;
			}
			// depth attachment reference
			if (spec.EnableDepthStencilAttachment)
			{
				multiSampleDepthAttachmentRef.sType			= VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
				multiSampleDepthAttachmentRef.attachment	= attachmentIndex;
				multiSampleDepthAttachmentRef.layout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				multiSampleDepthAttachmentRef.aspectMask	= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

				attachmentIndex++;

				subpassDepthStencilResolve.sType							= VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE;
				subpassDepthStencilResolve.pDepthStencilResolveAttachment	= &depthAttachmentRef;
				subpassDepthStencilResolve.depthResolveMode					= VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
				subpassDepthStencilResolve.stencilResolveMode				= VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
			}
		}

		subpass.sType					= VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
		subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount	= static_cast<uint32_t>(colorAttachmentRefs.size());
		subpass.pColorAttachments		= spec.Samples > 1 ? multiSampleColorAttachmentRefs.data() : colorAttachmentRefs.data();
		subpass.pResolveAttachments		= spec.Samples > 1 ? colorAttachmentRefs.data() : nullptr;
		subpass.pDepthStencilAttachment = spec.Samples > 1 ? &multiSampleDepthAttachmentRef : &depthAttachmentRef;
		subpass.pNext					= spec.Samples > 1 ? &subpassDepthStencilResolve : nullptr;

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

	void VulkanRenderPass::CreateRenderPass()
	{
		std::vector<VkAttachmentDescription2>				attachments;
		std::vector<VkAttachmentReference2>					colorAttachmentRefs;
		std::vector<VkAttachmentReference2>					multiSampleColorAttachmentRefs;
		uint32_t											attachmentIndex = 0;
		VkAttachmentDescription2							description{};
		VkAttachmentReference2								ref{};
		VkAttachmentReference2								depthAttachmentRef{};
		VkAttachmentReference2								multiSampleDepthAttachmentRef{};
		VkSubpassDescriptionDepthStencilResolve				subpassDepthStencilResolve{};
		std::vector<VkSubpassDescription2>					subpasses;
		VkSubpassDescription2								subpass{};
		std::vector<std::vector<VkAttachmentReference2>>	subpassInputAttachmentRefs(m_Specification.Subpasses.size());
		std::vector<std::vector<VkAttachmentReference2>>	subpassColorAttachmentRefs(m_Specification.Subpasses.size());
		std::vector<std::vector<VkAttachmentReference2>>    subpassMultiSampleColorAttachmentRefs(m_Specification.Subpasses.size());
		std::vector<VkSubpassDependency2>					dependencies;
		VkSubpassDependency2								dependency{};
		// each color attachment. 1 sample
		for (int i = 0; i < m_Specification.RenderTargets.size(); i++)
		{
			description = Utils::CreateAttachmentDescription2(m_Specification.RenderTargets.at(i), VK_SAMPLE_COUNT_1_BIT,
																Utils::RenderPassBeginOperationToVkAttachmentLoadOp(m_Specification.Operation.ColorBegin),
																Utils::RenderPassEndOperationToVkAttachmentStoreOp(m_Specification.Operation.ColorEnd));
			attachments.push_back(description);
		}
		// only one depth stencil attachment is allowed
		if (m_Specification.DepthStencil != FRAME_BUFFER_TEXTURE_FORMAT_NONE)
		{
			description = Utils::CreateAttachmentDescription2(m_Specification.DepthStencil, VK_SAMPLE_COUNT_1_BIT,
																Utils::RenderPassBeginOperationToVkAttachmentLoadOp(m_Specification.Operation.DepthStencilBegin),
																Utils::RenderPassEndOperationToVkAttachmentStoreOp(m_Specification.Operation.DepthStencilEnd));
			attachments.push_back(description);
			
		}
		// multi sample attachments. each color and depth-stencil attachment has a multi sample attachment
		if (m_Specification.Samples > 1)
		{
			for (int i = 0; i < m_Specification.RenderTargets.size(); i++)
			{
				description = Utils::CreateAttachmentDescription2(m_Specification.RenderTargets.at(i), Utils::SampleCountToVulkanFlag(m_Specification.Samples),
																	Utils::RenderPassBeginOperationToVkAttachmentLoadOp(m_Specification.Operation.ColorBegin),
																	Utils::RenderPassEndOperationToVkAttachmentStoreOp(m_Specification.Operation.ColorEnd));
				attachments.push_back(description);
			}
			if (m_Specification.DepthStencil != FRAME_BUFFER_TEXTURE_FORMAT_NONE)
			{
				description = Utils::CreateAttachmentDescription2(m_Specification.DepthStencil, Utils::SampleCountToVulkanFlag(m_Specification.Samples),
																	Utils::RenderPassBeginOperationToVkAttachmentLoadOp(m_Specification.Operation.DepthStencilBegin),
																	Utils::RenderPassEndOperationToVkAttachmentStoreOp(m_Specification.Operation.DepthStencilEnd));
				attachments.push_back(description);
			}
		}

		// 1 sample color attachment reference
		for (int i = 0; i < m_Specification.RenderTargets.size(); i++)
		{
			ref = Utils::CreateAttachmentReference2(m_Specification.RenderTargets.at(i), attachmentIndex);
			colorAttachmentRefs.push_back(ref);
			attachmentIndex++;
		}
		// 1 sample depth stencil attachment reference
		if (m_Specification.DepthStencil != FRAME_BUFFER_TEXTURE_FORMAT_NONE)
		{
			depthAttachmentRef = Utils::CreateAttachmentReference2(m_Specification.DepthStencil, attachmentIndex);
			attachmentIndex++;
		}
		// multi sample color attachment reference
		if (m_Specification.Samples > 1)
		{
			// multi sample color attachment reference
			for (int i = 0; i < m_Specification.RenderTargets.size(); i++)
			{
				ref = Utils::CreateAttachmentReference2(m_Specification.RenderTargets.at(i), attachmentIndex);
				multiSampleColorAttachmentRefs.push_back(ref);
				attachmentIndex++;
			}
			// multi sample depth stencil attachment reference
			if (m_Specification.DepthStencil != FRAME_BUFFER_TEXTURE_FORMAT_NONE)
			{
				multiSampleDepthAttachmentRef = Utils::CreateAttachmentReference2(m_Specification.DepthStencil, attachmentIndex);
				
				subpassDepthStencilResolve.sType							= VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE;
				subpassDepthStencilResolve.pDepthStencilResolveAttachment	= &depthAttachmentRef;
				subpassDepthStencilResolve.depthResolveMode					= VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
				subpassDepthStencilResolve.stencilResolveMode				= VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
			}
		}
		// create subpasses
		if (m_Specification.Subpasses.size() > 0)
		{
			// use the subpasses specified in the specification
			for (int i = 0; i < m_Specification.Subpasses.size(); i++)
			{
				// preapre input attachment references for this subpass
				for (int j = 0; j < m_Specification.Subpasses.at(i).InputRenderTargetIndices.size(); j++)
				{
					uint32_t inputAttachmentIndex = m_Specification.Subpasses.at(i).InputRenderTargetIndices.at(j);
					// color input
					if (inputAttachmentIndex < colorAttachmentRefs.size())
					{
						if (m_Specification.Samples > 1)
						{
							// if multi sample is enabled, we need to use the multi sample color attachment reference as input attachment reference
							subpassInputAttachmentRefs.at(i).push_back(multiSampleColorAttachmentRefs.at(inputAttachmentIndex));
						}
						else
						{
							subpassInputAttachmentRefs.at(i).push_back(colorAttachmentRefs.at(inputAttachmentIndex));
						}
					}
					// depth stencil input
					// depth stencil index is the one after the last color attachment index
					else if (inputAttachmentIndex == colorAttachmentRefs.size() && m_Specification.DepthStencil != FRAME_BUFFER_TEXTURE_FORMAT_NONE)
					{
						if (m_Specification.Samples > 1)
						{
							subpassInputAttachmentRefs.at(i).push_back(multiSampleDepthAttachmentRef);
						}
						else
						{
							subpassInputAttachmentRefs.at(i).push_back(depthAttachmentRef);
						}
					}
				}
				// prepare color attachment references for this subpass
				for (int j = 0; j < m_Specification.Subpasses.at(i).RenderTargetIndices.size(); j++)
				{
					uint32_t colorAttachmentIndex = m_Specification.Subpasses.at(i).RenderTargetIndices.at(j);
					if (colorAttachmentIndex < colorAttachmentRefs.size())
					{
						subpassColorAttachmentRefs.at(i).push_back(colorAttachmentRefs.at(colorAttachmentIndex));
						// add multi sample color attachment reference for subpass info if multi sample is enabled
						subpassMultiSampleColorAttachmentRefs.at(i).push_back(multiSampleColorAttachmentRefs.at(colorAttachmentIndex));
					}
				}
				// create subpass description for this subpass
				subpass.sType					= VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
				subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
				subpass.flags					= 0;
				subpass.inputAttachmentCount	= static_cast<uint32_t>(subpassInputAttachmentRefs.at(i).size());
				subpass.pInputAttachments		= subpassInputAttachmentRefs.at(i).data();
				subpass.colorAttachmentCount	= static_cast<uint32_t>(subpassColorAttachmentRefs.at(i).size());
				subpass.pColorAttachments		= m_Specification.Samples > 1 ? subpassMultiSampleColorAttachmentRefs.at(i).data() : subpassColorAttachmentRefs.at(i).data();
				subpass.pResolveAttachments		= m_Specification.Samples > 1 ? subpassColorAttachmentRefs.at(i).data() : VK_NULL_HANDLE;
				subpass.pDepthStencilAttachment = m_Specification.Subpasses.at(i).EnableDepthStencil ? (m_Specification.Samples > 1 ? &multiSampleDepthAttachmentRef : &depthAttachmentRef) : VK_NULL_HANDLE;
				subpass.pNext					= m_Specification.Subpasses.at(i).EnableDepthStencil && m_Specification.Samples > 1 ? &subpassDepthStencilResolve : VK_NULL_HANDLE;

				subpasses.push_back(subpass);
			}
		}
		else
		{
			// use default subpass
			subpass.sType					= VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
			subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.flags					= 0;
			subpass.inputAttachmentCount	= 0;
			subpass.colorAttachmentCount	= static_cast<uint32_t>(colorAttachmentRefs.size());
			subpass.pColorAttachments		= m_Specification.Samples > 1 ? multiSampleColorAttachmentRefs.data() : colorAttachmentRefs.data();
			subpass.pResolveAttachments		= m_Specification.Samples > 1 ? colorAttachmentRefs.data() : VK_NULL_HANDLE;
			subpass.pDepthStencilAttachment = m_Specification.Samples > 1 ? &multiSampleDepthAttachmentRef : &depthAttachmentRef;
			subpass.pNext					= m_Specification.Samples > 1 ? &subpassDepthStencilResolve : VK_NULL_HANDLE;

			subpasses.push_back(subpass);
		}	
		// create subpass dependencies
		if (m_Specification.SubpassDependencies.size() > 0)
		{
			for (int i = 0; i < m_Specification.SubpassDependencies.size(); i++)
			{
				// create subpass dependencies based on the specification
				dependency.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
				dependency.srcSubpass = m_Specification.SubpassDependencies.at(i).SrcSubpass;
				dependency.dstSubpass = m_Specification.SubpassDependencies.at(i).DstSubpass;
				// for temporary solution, 
				// we will use the subpass dependency specification to set the stage and access masks. 
				// In the future, we can implement a more sophisticated system to automatically determine the appropriate stage and access masks based on the subpass configuration.
				dependency.srcStageMask = m_Specification.SubpassDependencies.at(i).SrcStageMask;
				dependency.srcAccessMask = m_Specification.SubpassDependencies.at(i).SrcAccessMask;
				dependency.dstStageMask = m_Specification.SubpassDependencies.at(i).DstStageMask;
				dependency.dstAccessMask = m_Specification.SubpassDependencies.at(i).DstAccessMask;

				dependencies.push_back(dependency);
			}
		}
		else
		{
			// use default subpass dependency
			dependency.sType		= VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
			dependency.srcSubpass	= VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass	= 0;
			if (m_Specification.RenderTargets.size() > 0)
			{
				dependency.srcStageMask		|= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency.srcAccessMask	|= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				dependency.dstStageMask		|= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency.dstAccessMask	|= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			}
			if (m_Specification.DepthStencil != FRAME_BUFFER_TEXTURE_FORMAT_NONE)
			{
				dependency.srcStageMask		|= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				dependency.srcAccessMask	|= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				dependency.dstStageMask		|= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				dependency.dstAccessMask	|= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			}

			dependencies.push_back(dependency);
		}

		VkRenderPassCreateInfo2          renderPassInfo{};
		renderPassInfo.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
		renderPassInfo.attachmentCount	= static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments		= attachments.data();
		renderPassInfo.subpassCount		= static_cast<uint32_t>(subpasses.size());
		renderPassInfo.pSubpasses		= subpasses.data();
		renderPassInfo.dependencyCount	= static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies	= dependencies.data();

		VK_CHECK_RESULT(vkCreateRenderPass2(VulkanContext::Get()->GetDevice(), &renderPassInfo, nullptr, &m_RenderPass));
	}

}