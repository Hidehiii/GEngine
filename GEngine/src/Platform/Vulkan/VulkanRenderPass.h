#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/RenderPass.h"
#include <vulkan/vulkan.h>

namespace GEngine
{
	struct RenderPassSpecificationForVulkan
	{
		std::vector<VkFormat>						ColorAttachmentsFormat;
		std::vector<VkImageLayout>					ColorAttachmentsFinalLayout;
		bool 										EnableDepthStencilAttachment;
		int											Samples = 1;
		VkAttachmentLoadOp							ColorBegin	= VK_ATTACHMENT_LOAD_OP_CLEAR;
		VkAttachmentStoreOp							ColorEnd	= VK_ATTACHMENT_STORE_OP_STORE;
		VkAttachmentLoadOp							DepthStencilrBegin	= VK_ATTACHMENT_LOAD_OP_CLEAR;
		VkAttachmentStoreOp							DepthStencilEnd		= VK_ATTACHMENT_STORE_OP_STORE;
	};

	class GENGINE_API VulkanRenderPass : public RenderPass
	{
	public:
		VulkanRenderPass(const RenderPassSpecification& spec);
		VulkanRenderPass(const RenderPassSpecificationForVulkan& spec);
		virtual ~VulkanRenderPass() override;

		VkRenderPass						GetRenderPass() { return m_RenderPass; }

		static Ref<VulkanRenderPass> Create(const RenderPassSpecificationForVulkan& spec);
	private:
		void CreateRenderPass(const RenderPassSpecification& spec);
		
	private:
		VkRenderPass						m_RenderPass;
	};
}


