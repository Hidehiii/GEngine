#include "GEpch.h"
#include "VulkanFrameBuffer.h"
#include "Platform/Vulkan/VulkanUtils.h"
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

			VK_CHECK_RESULT(vkCreateImage(VulkanContext::GetDevice(), &imageInfo, nullptr, &outImage));
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

			VK_CHECK_RESULT(vkCreateImageView(VulkanContext::GetDevice(), &createInfo, nullptr, &outImageView));
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
		CreateRenderPass();
		CreateBuffer();
	}
	VulkanFrameBuffer::~VulkanFrameBuffer()
	{
		vkDestroyFramebuffer(VulkanContext::GetDevice(), m_FrameBuffer, nullptr);
	}
	void VulkanFrameBuffer::Begin()
	{
		GE_CORE_ASSERT(VulkanContext::GetCurrentCommandBuffer(), "There is no commandbuffer be using");

		VkRenderPassBeginInfo					renderPassInfo{};
		renderPassInfo.sType					= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass				= m_RenderPass->GetRenderPass();
		renderPassInfo.framebuffer				= m_FrameBuffer;
		renderPassInfo.renderArea.offset		= { 0, 0 };
		renderPassInfo.renderArea.extent.width	= m_Specification.Width;
		renderPassInfo.renderArea.extent.height	= m_Specification.Height;

		VkClearValue							clearColor = { {{VulkanContext::GetClearColor().r,
																VulkanContext::GetClearColor().g,
																VulkanContext::GetClearColor().b,
																VulkanContext::GetClearColor().a}} };
		renderPassInfo.clearValueCount			= 1;
		renderPassInfo.pClearValues				= &clearColor;
		vkCmdBeginRenderPass(VulkanContext::GetCurrentCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		s_CurrentFrameBuffer = this;
	}
	void VulkanFrameBuffer::End()
	{
		vkCmdEndRenderPass(VulkanContext::GetCurrentCommandBuffer());
		s_CurrentFrameBuffer = nullptr;
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
	void VulkanFrameBuffer::CreateRenderPass()
	{
		VulkanRenderPassSpecification		spec;
		spec.ColorAttachments				= m_ColorAttachmentsSpecs;
		spec.DepthAttachment				= m_DepthAttachmentSpec;
		m_RenderPass = CreateRef<VulkanRenderPass>(spec);
	}
	void VulkanFrameBuffer::CreateBuffer()
	{
		if (m_FrameBuffer)
		{
			vkDestroyFramebuffer(VulkanContext::GetDevice(), m_FrameBuffer, nullptr);
			m_Attachments.clear();
		}

		for (int i = 0; i < m_ColorAttachmentsSpecs.size(); i++)
		{
			VkImage						image;
			VkImageView					imageView;
			switch (m_ColorAttachmentsSpecs.at(i).TextureFormat)
			{
			case FrameBufferTextureFormat::RGBA8:
				Utils::CreateImages(m_Specification.Width, m_Specification.Height, VK_FORMAT_R8G8B8A8_UNORM, 1, image);
				Utils::CreateImageViews(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, imageView);
				break;
			default:
				GE_CORE_ASSERT(false, "Unknown format");
				break;
			}
			m_Attachments.push_back(imageView);
		}
		VkImage							image;
		VkImageView						imageView;
		switch (m_DepthAttachmentSpec.TextureFormat)
		{
		case FrameBufferTextureFormat::DEPTH24STENCIL8:
			Utils::CreateImages(m_Specification.Width, m_Specification.Height, VK_FORMAT_D24_UNORM_S8_UINT, 1, image);
			Utils::CreateImageViews(image, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_ASPECT_DEPTH_BIT, imageView);
			break;
		case FrameBufferTextureFormat::DEPTH:
			Utils::CreateImages(m_Specification.Width, m_Specification.Height, VK_FORMAT_D32_SFLOAT, 1, image);
			Utils::CreateImageViews(image, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT, imageView);
			break;
		default:
			GE_CORE_ASSERT(false, "Unknown format");
			break;
		}
		m_Attachments.push_back(imageView);

		VkFramebufferCreateInfo			framebufferInfo{};
		framebufferInfo.sType			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass		= m_RenderPass->GetRenderPass();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(m_Attachments.size());
		framebufferInfo.pAttachments	= m_Attachments.data();
		framebufferInfo.width			= m_Specification.Width;
		framebufferInfo.height			= m_Specification.Height;
		framebufferInfo.layers			= 1;

		VK_CHECK_RESULT(vkCreateFramebuffer(VulkanContext::GetDevice(), &framebufferInfo, nullptr, &m_FrameBuffer));
	}
}