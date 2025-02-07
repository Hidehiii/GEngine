#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/RenderPass.h"
#include <vulkan/vulkan.h>

namespace GEngine
{
	struct RenderPassSpecificationForVulkan
	{
		std::vector<VkFormat>						ColorAttachmentsFormat;
		std::vector<VkImageLayout>					ColorAttachmentsFinalLayout;
		bool 										EnableDepthStencilAttachment;
		int											Samples = 1;
		VkAttachmentLoadOp							AttachmentsBeginAction	= VK_ATTACHMENT_LOAD_OP_CLEAR;
		VkAttachmentStoreOp							AttachmentsEndAction	= VK_ATTACHMENT_STORE_OP_STORE;
	};

	class GENGINE_API VulkanRenderPass : public RenderPass
	{
	public:
		VulkanRenderPass(const RenderPassSpecification& spec);
		VulkanRenderPass(const RenderPassSpecificationForVulkan& spec);
		virtual ~VulkanRenderPass() override;

		VkRenderPass						GetRenderPass() { return m_RenderPass; }
		RenderPassSpecificationForVulkan	GetSpecificationForVulkan() { return m_SpecificationForVulkan; }

		static Ref<VulkanRenderPass> Create(const RenderPassSpecificationForVulkan& spec);
	private:
		void CreateRenderPass(const RenderPassSpecification& spec);
		
	private:
		VkRenderPass						m_RenderPass;
		RenderPassSpecificationForVulkan	m_SpecificationForVulkan;
	};
}


