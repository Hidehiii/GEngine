#include "GEpch.h"
#include "VulkanFrameBuffer.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/Vulkan/VulkanTexture2D.h"

namespace GEngine
{

	VulkanFrameBuffer::VulkanFrameBuffer(const Ref<RenderPass>& renderPass, uint32_t width, uint32_t height)
	{
		m_Specification.ColorRTs				= renderPass->GetSpecification().ColorRTs;
		m_Specification.DepthStencil			= renderPass->GetSpecification().DepthStencil;
		m_Specification.Samples					= renderPass->GetSpecification().Samples;
		m_Specification.Width					= width;
		m_Specification.Height					= height;

		m_RenderPass = std::dynamic_pointer_cast<VulkanRenderPass>(renderPass);

		CreateBuffer();
	}
	VulkanFrameBuffer::VulkanFrameBuffer(const Ref<FrameBuffer>& buffer, uint32_t width, uint32_t height)
	{
		m_Specification			= buffer->GetSpecification();
		m_Specification.Width	= width;
		m_Specification.Height	= height;

		m_RenderPass = std::dynamic_pointer_cast<VulkanRenderPass>(buffer->GetRenderPass());

		CreateBuffer();
	}
	VulkanFrameBuffer::VulkanFrameBuffer(const Ref<VulkanRenderPass>& renderPass, const FrameBufferSpecificationForVulkan spec, const RenderPassSpecificationForVulkan& renderpassSpec)
	{

		m_Specification.Width					= spec.Width;
		m_Specification.Height					= spec.Height;
		m_Specification.Samples					= renderpassSpec.Samples;

		m_RenderPass = std::dynamic_pointer_cast<VulkanRenderPass>(renderPass);

		for (int i = 0; i < spec.ColorAttachments.size(); i++)
		{
			VkFormat						colorFormat = renderpassSpec.ColorAttachmentsFormat.at(i);
			m_Images.push_back(spec.ColorImages.at(i));
			m_Attachments.push_back(spec.ColorAttachments.at(i));
			m_ColorImageViews.push_back(spec.ColorAttachments.at(i));
			m_ImagesMemory.push_back(nullptr);
			if (m_Specification.Samples > 1)
			{
				VkImage						tempImage;
				VkImageView					tempImageView;
				VkDeviceMemory				tempImageMemory;
				Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
					VulkanContext::Get()->GetDevice(),
					m_Specification.Width,
					m_Specification.Height,
					colorFormat,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
					VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
					Utils::SampleCountToVulkanFlag(m_Specification.Samples),
					1,
					0,
					1,
					tempImage,
					tempImageMemory);
				Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), tempImage, colorFormat, VK_IMAGE_VIEW_TYPE_2D, 1, VK_IMAGE_ASPECT_COLOR_BIT, 1, tempImageView);
				m_Images.push_back(tempImage);
				m_Attachments.push_back(tempImageView);
				m_ImagesMemory.push_back(tempImageMemory);
			}
		}
		if (renderpassSpec.EnableDepthStencilAttachment)
		{
			VkDeviceMemory					imageMemory;
			VkImage							image;
			VkImageView						imageView;
			VkFormat						depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
			VkFlags							depthAspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
				VulkanContext::Get()->GetDevice(),
				m_Specification.Width,
				m_Specification.Height,
				depthFormat,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				VK_SAMPLE_COUNT_1_BIT,
				1,
				0,
				1,
				image,
				imageMemory);
			Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), image, depthFormat, VK_IMAGE_VIEW_TYPE_2D, 1, depthAspectFlag, 1, imageView);

			m_DepthStencilImage = image;
			m_DepthStencilImageView = imageView;
			m_DepthStencilImageMemory = imageMemory;

			m_Images.push_back(image);
			m_Attachments.push_back(imageView);
			m_ImagesMemory.push_back(imageMemory);
			if (m_Specification.Samples > 1)
			{
				VkImage						tempImage;
				VkImageView					tempImageView;
				VkDeviceMemory				tempImageMemory;
				Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
					VulkanContext::Get()->GetDevice(),
					m_Specification.Width,
					m_Specification.Height,
					depthFormat,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
					VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
					Utils::SampleCountToVulkanFlag(m_Specification.Samples),
					1,
					0,
					1,
					tempImage,
					tempImageMemory);
				Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), tempImage, depthFormat, VK_IMAGE_VIEW_TYPE_2D, 1, depthAspectFlag, 1, tempImageView);
				m_Images.push_back(tempImage);
				m_Attachments.push_back(tempImageView);
				m_ImagesMemory.push_back(tempImageMemory);
			}
			//m_DepthStencilRT = CreateRef<VulkanTexture2D>(depthFormat, m_DepthStencilImage, m_DepthStencilImageView, m_DepthStencilImageMemory, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, depthAspectFlag);
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
		if (VulkanContext::Get()->GetDevice())
		{
			vkQueueWaitIdle(VulkanContext::Get()->GetGraphicsQueue());
			vkDestroyFramebuffer(VulkanContext::Get()->GetDevice(), m_FrameBuffer, nullptr);
			if (m_Specification.Samples > 1)
			{
				for (int i = 1; i < m_Attachments.size(); i += 2)
				{
					vkDestroyImageView(VulkanContext::Get()->GetDevice(), m_Attachments.at(i), nullptr);
				}
				for (int i = 1; i < m_Images.size(); i += 2)
				{
					vkDestroyImage(VulkanContext::Get()->GetDevice(), m_Images.at(i), nullptr);
				}
				for (int i = 1; i < m_ImagesMemory.size(); i += 2)
				{
					vkFreeMemory(VulkanContext::Get()->GetDevice(), m_ImagesMemory.at(i), nullptr);
				}
			}
		}
		
	}
	void VulkanFrameBuffer::SetRenderPassOperation(const RenderPassOperation& op)
	{
		auto spec		= m_RenderPass->GetSpecification();
		spec.Operation	= op;
		m_RenderPass	= std::dynamic_pointer_cast<VulkanRenderPass>(RenderPass::Create(spec));
	}
	void VulkanFrameBuffer::Begin(CommandBuffer* cmdBuffer)
	{
		VkCommandBuffer cmd = ((VulkanCommandBuffer*)cmdBuffer)->GetCommandBuffer();
		for (int i = 0; i < m_ColorRTs.size(); i++)
		{
			m_ColorRTs.at(i)->SetImageLayout(cmd, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		}
		if (m_DepthStencil)
		{
			m_DepthStencil->SetImageLayout(cmd, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		}

		VkRenderPassBeginInfo					renderPassInfo{};
		renderPassInfo.sType					= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass				= m_RenderPass->GetRenderPass();
		renderPassInfo.framebuffer				= m_FrameBuffer;
		renderPassInfo.renderArea.offset		= { 0, 0 };
		renderPassInfo.renderArea.extent.width	= m_Specification.Width;
		renderPassInfo.renderArea.extent.height = m_Specification.Height;

		Vector4									setClearColor = VulkanContext::Get()->GetClearColor();
		VkClearValue							clearColor = {};
		clearColor.color						= { { setClearColor.r, setClearColor.g, setClearColor.b, setClearColor.a} };

		std::vector<VkClearValue>				clearValues(m_Attachments.size(), clearColor);
		if (m_DepthStencilImage != nullptr)
		{
			clearValues.at(clearValues.size() - 1) = { Graphics::IsReverseDepth() ? 0 : 1.0f, 0 };
		}
		renderPassInfo.clearValueCount			= static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues				= clearValues.data();
		vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}
	void VulkanFrameBuffer::End(CommandBuffer* cmdBuffer)
	{
		VkCommandBuffer cmd = ((VulkanCommandBuffer*)cmdBuffer)->GetCommandBuffer();
		vkCmdEndRenderPass(cmd);

		for (int i = 0; i < m_ColorRTs.size(); i++)
		{
			m_ColorRTs.at(i)->SetImageLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		if (m_DepthStencil)
		{
			m_DepthStencil->SetImageLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	}
	Ref<Texture2D> VulkanFrameBuffer::GetColorRT(int index)
	{
		GE_CORE_ASSERT(index < m_ColorImages.size(), "index out of range");
		Ref<VulkanTexture2D> texture = m_ColorRTs.at(index);
		return texture;
	}
	Ref<Texture2D> VulkanFrameBuffer::GetDepthStencil()
	{
		GE_CORE_ASSERT(m_DepthStencilImageView != nullptr, "no depth frame buffer");
		Ref<VulkanTexture2D> texture = m_DepthStencil;
		return texture;
	}
	Ref<VulkanFrameBuffer> VulkanFrameBuffer::Create(const Ref<VulkanRenderPass>& renderPass, const FrameBufferSpecificationForVulkan spec, const RenderPassSpecificationForVulkan& renderpassSpec)
	{
		return CreateRef<VulkanFrameBuffer>(renderPass, spec, renderpassSpec);
	}
	void VulkanFrameBuffer::CreateBuffer()
	{

		for (int i = 0; i < m_Specification.ColorRTs.size(); i++)
		{
			VkImage						image;
			VkImageView					imageView;
			VkDeviceMemory				imageMemory;
			VkFormat					colorFormat = Utils::FrameBufferTextureFormatToVulkanFormat(m_Specification.ColorRTs.at(i).TextureFormat);
			Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
				VulkanContext::Get()->GetDevice(),
				m_Specification.Width,
				m_Specification.Height,
				colorFormat,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				VK_SAMPLE_COUNT_1_BIT,
				1,
				0,
				1,
				image,
				imageMemory);
			Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), image, colorFormat, VK_IMAGE_VIEW_TYPE_2D, 1, VK_IMAGE_ASPECT_COLOR_BIT, 1, imageView);
			m_Images.push_back(image);
			m_Attachments.push_back(imageView);
			m_ImagesMemory.push_back(imageMemory);
			if (m_Specification.Samples > 1)
			{
				VkImage						tempImage;
				VkImageView					tempImageView;
				VkDeviceMemory				tempImageMemory;
				Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
					VulkanContext::Get()->GetDevice(),
					m_Specification.Width,
					m_Specification.Height,
					colorFormat,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					Utils::SampleCountToVulkanFlag(m_Specification.Samples),
					1,
					0,
					1,
					tempImage,
					tempImageMemory);
				Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), tempImage, colorFormat, VK_IMAGE_VIEW_TYPE_2D, 1, VK_IMAGE_ASPECT_COLOR_BIT, 1, tempImageView);
				m_Images.push_back(tempImage);
				m_Attachments.push_back(tempImageView);
				m_ImagesMemory.push_back(tempImageMemory);
			}

			m_ColorImages.push_back(image);
			m_ColorImageViews.push_back(imageView);
			m_ColorImagesMemory.push_back(imageMemory);

			Ref<VulkanTexture2D> texture = CreateRef<VulkanTexture2D>(colorFormat, m_ColorImages[i], m_ColorImageViews[i], m_ColorImagesMemory[i], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
			m_ColorRTs.push_back(texture);
		}
		VkDeviceMemory					imageMemory;
		VkImage							image;
		VkImageView						imageView;
		VkFormat						depthFormat;
		VkFlags							depthAspectFlag;
		switch (m_Specification.DepthStencil.TextureFormat)
		{
		case FrameBufferTextureFormat::DEPTH24STENCIL8:
		{
			depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
			depthAspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			break;
		}
		case FrameBufferTextureFormat::DEPTH:
		{
			depthFormat = VK_FORMAT_D32_SFLOAT;
			depthAspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
			break;
		}
		default:
			GE_CORE_ASSERT(false, "Unknown format");
			break;
		}
		Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
			VulkanContext::Get()->GetDevice(),
			m_Specification.Width,
			m_Specification.Height,
			depthFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			1,
			0,
			1,
			image,
			imageMemory);
		Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), image, depthFormat, VK_IMAGE_VIEW_TYPE_2D, 1, depthAspectFlag, 1, imageView);
		m_Images.push_back(image);
		m_Attachments.push_back(imageView);
		m_ImagesMemory.push_back(imageMemory);
		if (m_Specification.Samples > 1)
		{
			VkImage						tempImage;
			VkImageView					tempImageView;
			VkDeviceMemory				tempImageMemory;
			Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
				VulkanContext::Get()->GetDevice(),
				m_Specification.Width,
				m_Specification.Height,
				depthFormat,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				Utils::SampleCountToVulkanFlag(m_Specification.Samples),
				1,
				0,
				1,
				tempImage,
				tempImageMemory);
			Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), tempImage, depthFormat, VK_IMAGE_VIEW_TYPE_2D, 1, depthAspectFlag, 1, tempImageView);
			m_Images.push_back(tempImage);
			m_Attachments.push_back(tempImageView);
			m_ImagesMemory.push_back(tempImageMemory);
		}

		m_DepthStencilImage = image;
		m_DepthStencilImageView = imageView;
		m_DepthStencilImageMemory = imageMemory;

		m_DepthStencil = CreateRef<VulkanTexture2D>(depthFormat, m_DepthStencilImage, m_DepthStencilImageView, m_DepthStencilImageMemory , VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, depthAspectFlag);

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