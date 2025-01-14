#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/FrameBuffer.h"
#include <vulkan/vulkan.h>

namespace GEngine
{
	struct RenderPassSpecification
	{
		std::vector<FrameBufferTextureSpecification>	ColorAttachments;
		FrameBufferTextureSpecification					DepthAttachment;
		int												Samples = 1;
		FrameBufferAttachmentsAction					AttachmentsBeginAction = FrameBufferAttachmentsAction::Clear;
		FrameBufferAttachmentsAction					AttachmentsEndAction = FrameBufferAttachmentsAction::Store;
	};

	struct RenderPassSpecificationForVulkan
	{
		std::vector<VkFormat>						ColorAttachmentsFormat;
		std::vector<VkImageLayout>					ColorAttachmentsFinalLayout;
		bool 										EnableDepthStencilAttachment;
		int											Samples = 1;
		VkAttachmentLoadOp							AttachmentsBeginAction = VK_ATTACHMENT_LOAD_OP_CLEAR;
		VkAttachmentStoreOp							AttachmentsEndAction = VK_ATTACHMENT_STORE_OP_STORE;
	};

	class GENGINE_API VulkanRenderPass
	{
	public:
		VulkanRenderPass(const RenderPassSpecification& spec);
		VulkanRenderPass(const RenderPassSpecificationForVulkan& spec);
		~VulkanRenderPass();
		VkRenderPass GetRenderPass() { return m_RenderPass; }

		static Ref<VulkanRenderPass> Create(const RenderPassSpecificationForVulkan& spec);
	private:
		void CreateRenderPass(const RenderPassSpecification& spec);
		
	private:
		VkRenderPass						m_RenderPass;
	};
}


