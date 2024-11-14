#include "GEpch.h"
#include "VulkanFrameBuffer.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanTexture2D.h"

namespace GEngine
{
	VulkanFrameBuffer* VulkanFrameBuffer::s_CurrentVulkanFrameBuffer = nullptr;

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
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				image,
				imageMemory);
			Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), image, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, imageView);

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
		GE_CORE_ASSERT(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), "There is no commandbuffer be using");

		for (int i = 0; i < m_ColorAttachmentsTexture2D.size(); i++)
		{
			m_ColorAttachmentsTexture2D.at(i)->SetImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		}
		if (m_DepthAttachmentTexture2D)
		{
			m_DepthAttachmentTexture2D->SetImageLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		}

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
		vkCmdBeginRenderPass(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		s_CurrentVulkanFrameBuffer = this;
	}
	void VulkanFrameBuffer::End()
	{
		vkCmdEndRenderPass(VulkanContext::Get()->GetCurrentDrawCommandBuffer());
		s_CurrentVulkanFrameBuffer = nullptr;
	}
	Ref<Texture2D> VulkanFrameBuffer::GetColorAttachment(int index)
	{
		GE_CORE_ASSERT(index < m_ColorImages.size(), "index out of range");
		Ref<VulkanTexture2D> texture = m_ColorAttachmentsTexture2D.at(index);
		texture->SetImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		return texture;
	}
	Ref<Texture2D> VulkanFrameBuffer::GetDepthAttachment()
	{
		GE_CORE_ASSERT(m_DepthStencilImageView != nullptr, "no depth frame buffer");
		Ref<VulkanTexture2D> texture = m_DepthAttachmentTexture2D;
		texture->SetImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		return texture;
	}
	void VulkanFrameBuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width <= 0 || height <= 0)
		{
			GE_CORE_WARN("Attempted to resize framebuffer to {0}, {1}", width, height);
			return;
		}

		m_Specification.Width = width;
		m_Specification.Height = height;
		CreateBuffer();
	}
	void VulkanFrameBuffer::Resize(Vector2 size)
	{
		Resize((uint32_t)size.x, (uint32_t)size.y);
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

			m_Images.clear();
			m_Attachments.clear();
			m_ImagesMemory.clear();
			m_ColorImages.clear();
			m_ColorImageViews.clear();
			m_ColorImagesMemory.clear();
		}

		for (int i = 0; i < m_ColorAttachmentsSpecs.size(); i++)
		{
			VkImage						image;
			VkImageView					imageView;
			VkDeviceMemory				imageMemory;
			VkFormat					colorFormat;
			switch (m_ColorAttachmentsSpecs.at(i).TextureFormat)
			{
			case FrameBufferTextureFormat::RGBA8:
				colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
				Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
					VulkanContext::Get()->GetDevice(),
					m_Specification.Width, 
					m_Specification.Height, 
					colorFormat,
					VK_IMAGE_TILING_OPTIMAL, 
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					image,
					imageMemory);
				Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), image, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, imageView);
				break;
			default:
				GE_CORE_ASSERT(false, "Unknown format");
				break;
			}
			m_Images.push_back(image);
			m_Attachments.push_back(imageView);
			m_ImagesMemory.push_back(imageMemory);

			m_ColorImages.push_back(image);
			m_ColorImageViews.push_back(imageView);
			m_ColorImagesMemory.push_back(imageMemory);

			Ref<VulkanTexture2D> texture = CreateRef<VulkanTexture2D>(colorFormat, m_ColorImages[i], m_ColorImageViews[i], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, true);
			m_ColorAttachmentsTexture2D.push_back(texture);
		}
		VkDeviceMemory					imageMemory;
		VkImage							image;
		VkImageView						imageView;
		VkFormat						depthFormat;
		VkFlags							depthAspectFlag;
		switch (m_DepthAttachmentSpec.TextureFormat)
		{
		case FrameBufferTextureFormat::DEPTH24STENCIL8:
		{
			depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
			depthAspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
				VulkanContext::Get()->GetDevice(),
				m_Specification.Width, 
				m_Specification.Height, 
				depthFormat,
				VK_IMAGE_TILING_OPTIMAL, 
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				image,
				imageMemory);
			Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), image, depthFormat, depthAspectFlag, imageView);
			break;
		}
			
		case FrameBufferTextureFormat::DEPTH:
			depthFormat = VK_FORMAT_D32_SFLOAT;
			depthAspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
			Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
				VulkanContext::Get()->GetDevice(),
				m_Specification.Width, 
				m_Specification.Height, 
				depthFormat,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				image, 
				imageMemory);
			Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), image, depthFormat, depthAspectFlag, imageView);
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

		m_DepthAttachmentTexture2D = CreateRef<VulkanTexture2D>(depthFormat, m_DepthStencilImage, m_DepthStencilImageView, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, depthAspectFlag, true);

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