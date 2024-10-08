#include "GEpch.h"
#include "VulkanFrameBuffer.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace GEngine
{
	namespace Utils
	{
		static void CreateImages(uint32_t width, uint32_t height, VkFormat format, int samples, VkImage& outImage)
		{
			VkImageCreateInfo		imageInfo{};
			imageInfo.sType			= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType		= VK_IMAGE_TYPE_2D;
			imageInfo.extent.width	= width;
			imageInfo.extent.height = height;
			imageInfo.extent.depth	= 1;
			imageInfo.mipLevels		= 1;
			imageInfo.arrayLayers	= 1;
			imageInfo.format		= format;
			imageInfo.tiling		= VK_IMAGE_TILING_OPTIMAL;
			imageInfo.samples		= VK_SAMPLE_COUNT_1_BIT;
			imageInfo.flags			= 0; // Optional

			if (vkCreateImage(VulkanContext::GetDevice(), &imageInfo, nullptr, &outImage) != VK_SUCCESS) {
				GE_CORE_ERROR("failed to create image!");
			}
		}

		static void CreateImageViews(VkImage image, VkFormat format, VkImageAspectFlags aspectMask, VkImageView& outImageView)
		{
			VkImageViewCreateInfo                           createInfo{};
			createInfo.sType								= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image								= image;
			createInfo.viewType								= VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format								= format;
			createInfo.components.r							= VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g							= VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b							= VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a							= VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask			= aspectMask;
			createInfo.subresourceRange.baseMipLevel		= 0;
			createInfo.subresourceRange.levelCount			= 1;
			createInfo.subresourceRange.baseArrayLayer		= 0;
			createInfo.subresourceRange.layerCount			= 1;

			if (vkCreateImageView(VulkanContext::GetDevice(), &createInfo, nullptr, &outImageView) != VK_SUCCESS)
			{
				GE_CORE_ERROR("failed to create image views!");
			}
		}
	}


	VulkanFrameBuffer::VulkanFrameBuffer(const FrameBufferSpecification& spec)
	{
		m_Specification = spec;
		for (auto format : m_Specification.Attachments.Attachments)
		{
			if (Utils::isDepthFormat(format.TextureFormat))
			{
				m_DepthAttachmentSpec = format;
			}
			else
			{
				m_ColorAttachmentsSpecs.emplace_back(format);
			}
		}

		CreateBuffer();
	}
	VulkanFrameBuffer::~VulkanFrameBuffer()
	{
		vkDestroyFramebuffer(VulkanContext::GetDevice(), m_FrameBuffer, nullptr);
	}
	void VulkanFrameBuffer::Bind()
	{
		VkRenderPassBeginInfo		renderPassInfo{};
		renderPassInfo.sType		= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass	= m_RenderPass->GetRenderPass();
		renderPassInfo.framebuffer	= m_FrameBuffer;
	}
	void VulkanFrameBuffer::Unbind()
	{
	}
	void VulkanFrameBuffer::Resize(uint32_t width, uint32_t height)
	{
	}
	void VulkanFrameBuffer::Resize(Vector2 size)
	{
	}
	int VulkanFrameBuffer::ReadPixelInt(int attachmentIndex, int x, int y)
	{
		return 0;
	}
	void VulkanFrameBuffer::ClearAttachmentInt(int attachmentIndex, int val)
	{
	}
	void VulkanFrameBuffer::CreateBuffer()
	{
		if (m_FrameBuffer)
		{
			vkDestroyFramebuffer(VulkanContext::GetDevice(), m_FrameBuffer, nullptr);
			m_Attachments.clear();
		}

		VkFramebufferCreateInfo			framebufferInfo{};
		framebufferInfo.sType			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass		= m_RenderPass->GetRenderPass();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(m_Attachments.size());
		framebufferInfo.pAttachments	= m_Attachments.data();
		framebufferInfo.width			= m_Specification.Width;
		framebufferInfo.height			= m_Specification.Height;
		framebufferInfo.layers			= 1;

		if (vkCreateFramebuffer(VulkanContext::GetDevice(), &framebufferInfo, nullptr, &m_FrameBuffer) != VK_SUCCESS) {
			GE_CORE_ERROR("failed to create framebuffer!");
		}
	}
}