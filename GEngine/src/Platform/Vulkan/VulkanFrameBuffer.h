#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/FrameBuffer.h"
#include "Platform/Vulkan/VulkanTexture2D.h"
#include "VulkanRenderPass.h"

namespace GEngine
{
	struct FrameBufferSpecificationForVulkan
	{
		std::vector<VkImageView>	ColorAttachments;
		std::vector<VkImage>		ColorImages;
		uint32_t					Width, Height;
		std::vector<VkFormat>		ColorAttachmentsFormat;
		std::vector<VkImageLayout>	ColorAttachmentsFinalLayout;
		bool						EnableDepthStencilAttachment = false;
		int							Samples = 1;
		VkAttachmentLoadOp			AttachmentsBeginAction = VK_ATTACHMENT_LOAD_OP_CLEAR;
		VkAttachmentStoreOp			AttachmentsEndAction = VK_ATTACHMENT_STORE_OP_STORE;
	};

	class GENGINE_API VulkanFrameBuffer : public FrameBuffer
	{
	public:
		VulkanFrameBuffer(const FrameBufferSpecification& spec);
		VulkanFrameBuffer(const Ref<FrameBuffer>& buffer, uint32_t width, uint32_t height);
		VulkanFrameBuffer(const FrameBufferSpecificationForVulkan spec);
		virtual ~VulkanFrameBuffer() override;

		virtual void Begin() override;
		virtual void End() override;

		virtual int								GetColorAttachmentCount() override { return m_ColorImageViews.size(); }
		virtual int								GetAttachmentCount() override { return m_Attachments.size(); }
		virtual Ref<Texture2D>					GetColorAttachment(int index) override;
		virtual Ref<Texture2D>					GetDepthStencilAttachment() override;
		virtual const FrameBufferSpecification& GetSpecification() const override { return m_Specification; }

		VkRenderPass	GetRenderPass() { return m_RenderPass->GetRenderPass(); }
		VkFramebuffer	GetFrameBuffer() { return m_FrameBuffer; }
	public:
		//用于给交换链创建使用
		static Ref<VulkanFrameBuffer> Create(const FrameBufferSpecificationForVulkan spec);

		static VulkanFrameBuffer* GetCurrentVulkanFrameBuffer() { return s_CurrentVulkanFrameBuffer; }
	private:
		void CreateRenderPass();
		void CreateBuffer();
	private:
		static VulkanFrameBuffer*	s_CurrentVulkanFrameBuffer;
		VkFramebuffer				m_FrameBuffer = nullptr;
		Ref<VulkanRenderPass>		m_RenderPass = nullptr;
		std::vector<VkImage>		m_Images;
		std::vector<VkImageView>	m_Attachments;
		std::vector<VkDeviceMemory> m_ImagesMemory;
		FrameBufferSpecificationForVulkan m_SpecificationForVulkan;
		std::vector<VkImage>		m_ColorImages;
		std::vector<VkImageView>	m_ColorImageViews;
		std::vector<VkDeviceMemory> m_ColorImagesMemory;
		VkImage						m_DepthStencilImage = nullptr;
		VkImageView					m_DepthStencilImageView = nullptr;
		VkDeviceMemory				m_DepthStencilImageMemory = nullptr;

		std::vector<Ref<VulkanTexture2D>> m_ColorAttachmentsTexture2D;
		Ref<VulkanTexture2D>			m_DepthAttachmentTexture2D = nullptr;
	};
}


