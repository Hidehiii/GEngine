#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/FrameBuffer.h"
#include "VulkanRenderPass.h"

namespace GEngine
{
	class GENGINE_API VulkanFrameBuffer : public FrameBuffer
	{
	public:
		VulkanFrameBuffer(const FrameBufferSpecification& spec);
		virtual ~VulkanFrameBuffer() override;

		VkRenderPass GetRenderPass() { return m_RenderPass->GetRenderPass(); }
		virtual void Begin() override;
		virtual void End() override;

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual void Resize(Vector2 size) override;

		virtual int ReadPixelInt(int attachmentIndex, int x, int y) override;
		virtual const FrameBufferSpecification& GetSpecification() const override { return m_Specification; }
		virtual void ClearAttachmentInt(int attachmentIndex, int val) override;
		virtual uint32_t GetColorAttachment(uint32_t index = 0) const override { return 0; }
		virtual uint32_t GetDepthAttachment() const override { return 0; }
	private:
		void CreateRenderPass();
		void CreateBuffer();
	private:
		VkFramebuffer				m_FrameBuffer = nullptr;
		Ref<VulkanRenderPass>		m_RenderPass = nullptr;
		std::vector<VkImage>		m_Images;
		std::vector<VkImageView>	m_Attachments;
		std::vector<VkDeviceMemory> m_ImagesMemory;
	};
}


