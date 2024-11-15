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
	};

	struct RenderPassSpecificationForVulkan
	{
		std::vector<VkFormat>						ColorAttachmentsFormat;
		std::vector<VkImageLayout>					ColorAttachmentsFinalLayout;
		bool 										EnableDepthStencilAttachment;
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


