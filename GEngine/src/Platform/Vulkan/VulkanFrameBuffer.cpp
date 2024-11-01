#include "GEpch.h"
#include "VulkanFrameBuffer.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace GEngine
{
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
	VulkanFrameBuffer::VulkanFrameBuffer(const FrameBufferSpecificationForVulkan spec)
	{
		m_SpecificationForVulkan		= spec;
		m_Specification.Width			= spec.Width;
		m_Specification.Height			= spec.Height;
		
		RenderPassSpecificationForVulkan renderPassSpec;
		renderPassSpec.ColorAttachmentsFinalLayout	= spec.ColorAttachmentsFinalLayout;
		renderPassSpec.ColorAttachmentsFormat		= spec.ColorAttachmentsFormat;
		renderPassSpec.EnableDepthStencilAttachment = spec.EnableDepthStencilAttachment;

		m_RenderPass = CreateRef<VulkanRenderPass>(renderPassSpec);

		m_Attachments = spec.ColorAttachments;
		if (spec.EnableDepthStencilAttachment)
		{
			VkDeviceMemory					imageMemory;
			VkImage							image;
			VkImageView						imageView;
			Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
				VulkanContext::Get()->GetDevice(),
				m_Specification.Width,
				m_Specification.Height,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				image,
				imageMemory);
			Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), image, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_ASPECT_DEPTH_BIT, imageView);

			m_DepthStencilImage = image;
			m_DepthStencilImageView = imageView;
			m_DepthStencilImageMemory = imageMemory;

			m_Attachments.push_back(imageView);
		}

		VkFramebufferCreateInfo         framebufferInfo{};
		framebufferInfo.sType			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass		= m_RenderPass->GetRenderPass();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(m_Attachments.size());
		framebufferInfo.pAttachments	= m_Attachments.data();
		framebufferInfo.width			= spec.Width;
		framebufferInfo.height			= spec.Height;
		framebufferInfo.layers			= 1;

		VK_CHECK_RESULT(vkCreateFramebuffer(VulkanContext::Get()->GetDevice(), &framebufferInfo, nullptr, &m_FrameBuffer));
	}
	VulkanFrameBuffer::~VulkanFrameBuffer()
	{
		vkDestroyFramebuffer(VulkanContext::Get()->GetDevice(), m_FrameBuffer, nullptr);
	}
	void VulkanFrameBuffer::Begin()
	{
		GE_CORE_ASSERT(VulkanContext::Get()->GetCurrentCommandBuffer(), "There is no commandbuffer be using");

		VkRenderPassBeginInfo					renderPassInfo{};
		renderPassInfo.sType					= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass				= m_RenderPass->GetRenderPass();
		renderPassInfo.framebuffer				= m_FrameBuffer;
		renderPassInfo.renderArea.offset		= { 0, 0 };
		renderPassInfo.renderArea.extent.width	= m_Specification.Width;
		renderPassInfo.renderArea.extent.height	= m_Specification.Height;

		Vector4									setClearColor = VulkanContext::Get()->GetClearColor();
		VkClearValue							clearColor = {};
		clearColor.color						= { { setClearColor.r, setClearColor.g, setClearColor.b, setClearColor.a} };

		std::vector<VkClearValue>				clearValues(m_Attachments.size(), clearColor);
		if (m_DepthStencilImage != nullptr)
		{
			clearValues.at(clearValues.size() - 1) = { 1.0f, 0 };
		}
		renderPassInfo.clearValueCount			= static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues				= clearValues.data();
		vkCmdBeginRenderPass(VulkanContext::Get()->GetCurrentCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		s_CurrentFrameBuffer = this;
	}
	void VulkanFrameBuffer::End()
	{
		vkCmdEndRenderPass(VulkanContext::Get()->GetCurrentCommandBuffer());
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
	Ref<VulkanFrameBuffer> VulkanFrameBuffer::Create(const FrameBufferSpecificationForVulkan spec)
	{
		return CreateRef<VulkanFrameBuffer>(spec);
	}
	void VulkanFrameBuffer::CreateRenderPass()
	{
		RenderPassSpecification				spec;
		spec.ColorAttachments				= m_ColorAttachmentsSpecs;
		spec.DepthAttachment				= m_DepthAttachmentSpec;
		m_RenderPass = CreateRef<VulkanRenderPass>(spec);
	}
	void VulkanFrameBuffer::CreateBuffer()
	{
		if (m_FrameBuffer)
		{
			vkDestroyFramebuffer(VulkanContext::Get()->GetDevice(), m_FrameBuffer, nullptr);
			m_Attachments.clear();
		}

		for (int i = 0; i < m_ColorAttachmentsSpecs.size(); i++)
		{
			VkImage						image;
			VkImageView					imageView;
			VkDeviceMemory				imageMemory;
			switch (m_ColorAttachmentsSpecs.at(i).TextureFormat)
			{
			case FrameBufferTextureFormat::RGBA8:
				Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
					VulkanContext::Get()->GetDevice(),
					m_Specification.Width, 
					m_Specification.Height, 
					VK_FORMAT_R8G8B8A8_UNORM, 
					VK_IMAGE_TILING_OPTIMAL, 
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					image,
					imageMemory);
				Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, imageView);
				break;
			default:
				GE_CORE_ASSERT(false, "Unknown format");
				break;
			}
			m_Images.push_back(image);
			m_Attachments.push_back(imageView);
			m_ImagesMemory.push_back(imageMemory);
		}
		VkDeviceMemory					imageMemory;
		VkImage							image;
		VkImageView						imageView;
		switch (m_DepthAttachmentSpec.TextureFormat)
		{
		case FrameBufferTextureFormat::DEPTH24STENCIL8:
		{
			Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
				VulkanContext::Get()->GetDevice(),
				m_Specification.Width, 
				m_Specification.Height, 
				VK_FORMAT_D24_UNORM_S8_UINT, 
				VK_IMAGE_TILING_OPTIMAL, 
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				image,
				imageMemory);
			Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), image, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_ASPECT_DEPTH_BIT, imageView);
			break;
		}
			
		case FrameBufferTextureFormat::DEPTH:
			Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
				VulkanContext::Get()->GetDevice(),
				m_Specification.Width, 
				m_Specification.Height, 
				VK_FORMAT_D32_SFLOAT, 
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				image, 
				imageMemory);
			Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), image, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT, imageView);
			break;
		default:
			GE_CORE_ASSERT(false, "Unknown format");
			break;
		}
		m_Images.push_back(image);
		m_Attachments.push_back(imageView);
		m_ImagesMemory.push_back(imageMemory);

		m_DepthStencilImage = image;
		m_DepthStencilImageView = imageView;
		m_DepthStencilImageMemory = imageMemory;

		VkFramebufferCreateInfo			framebufferInfo{};
		framebufferInfo.sType			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass		= m_RenderPass->GetRenderPass();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(m_Attachments.size());
		framebufferInfo.pAttachments	= m_Attachments.data();
		framebufferInfo.width			= m_Specification.Width;
		framebufferInfo.height			= m_Specification.Height;
		framebufferInfo.layers			= 1;

		VK_CHECK_RESULT(vkCreateFramebuffer(VulkanContext::Get()->GetDevice(), &framebufferInfo, nullptr, &m_FrameBuffer));
	}
}