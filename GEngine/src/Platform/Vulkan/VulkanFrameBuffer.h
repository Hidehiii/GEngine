#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/FrameBuffer.h"
#include "Platform/Vulkan/VulkanTexture2D.h"
#include "VulkanRenderPass.h"

namespace GEngine
{
	struct FrameBufferSpecificationForVulkan
	{
		std::vector<VkImageView>	ColorAttachments;
		std::vector<VkImage>		ColorImages;
		uint32_t					Width, Height;
	};

	class GENGINE_API VulkanFrameBuffer : public FrameBuffer
	{
	public:
		VulkanFrameBuffer(const Ref<RenderPass>& renderPass, uint32_t width, uint32_t height);
		VulkanFrameBuffer(const Ref<FrameBuffer>& buffer, uint32_t width, uint32_t height);
		VulkanFrameBuffer(const Ref<VulkanRenderPass>& renderPass, const FrameBufferSpecificationForVulkan spec, const RenderPassSpecificationForVulkan& renderpassSpec);
		virtual ~VulkanFrameBuffer() override;

		virtual void Begin(CommandBuffer* cmdBuffer) override;
		virtual void End(CommandBuffer* cmdBuffer) override;

		virtual void SetRenderPassOperation(const RenderPassOperation& op) override;

		virtual int								GetColorRTCount() override { return m_ColorImages.size(); }
		virtual int								GetRTCount() override { return m_Attachments.size(); }
		virtual Ref<Texture2D>					GetColorRT(int index) override;
		virtual Ref<Texture2D>					GetDepthStencil() override;
		virtual const FrameBufferSpecification& GetSpecification() const override { return m_Specification; }
		virtual	Ref<RenderPass>					GetRenderPass() override { return std::static_pointer_cast<RenderPass>(m_RenderPass); }

		VkFramebuffer	GetVulkanFrameBuffer() { return m_FrameBuffer; }
		VkRenderPass	GetVulkanRenderPass() { return m_RenderPass->GetRenderPass(); }
	public:
		//用于给交换链创建使用
		static Ref<VulkanFrameBuffer> CreateForSwapChain(const Ref<VulkanRenderPass>& renderPass, const FrameBufferSpecificationForVulkan spec, const RenderPassSpecificationForVulkan& renderpassSpec);
	private:
		void CreateBuffer();
	private:
		VkFramebuffer				m_FrameBuffer = nullptr;
		Ref<VulkanRenderPass>		m_RenderPass = nullptr;
		std::vector<VkImageView>	m_Attachments;
		std::vector<VkImageView>	m_SwapChainAdditionalAttachments;
		std::vector<VkImage>		m_ColorImages;
		std::vector<VkDeviceMemory> m_ColorImagesMemory;
		std::vector<VkImage>		m_DepthStencilImages;
		std::vector<VkDeviceMemory> m_DepthStencilImagesMemory;

		std::vector<Ref<VulkanTexture2D>>	m_ColorRTs;
		Ref<VulkanTexture2D>				m_DepthStencil = nullptr;
	};
}


