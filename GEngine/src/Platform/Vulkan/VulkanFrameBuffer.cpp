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
		m_Specification.RenderTargets			= renderPass->GetSpecification().RenderTargets;
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
		// restore the count of render targets to the specification, 
		// because the spec is only used to create the framebuffer, 
		// and the render targets are not used to create the framebuffer, 
		// so we need to restore the count of render targets to the specification.
		m_Specification.RenderTargets			= std::vector<FrameBufferTextureFormat>(renderpassSpec.ColorAttachmentsFormat.size(), FRAME_BUFFER_TEXTURE_FORMAT_NONE);

		m_RenderPass = std::dynamic_pointer_cast<VulkanRenderPass>(renderPass);

		std::vector<VkImageView>	attachments(spec.ColorAttachments);
		VkImage						image;
		VkImageView					imageView;
		VkDeviceMemory				imageMemory;
		VkFormat					imageFormat;
		VkFlags						depthAspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

		// create depth stencil image if needed
		if (renderpassSpec.EnableDepthStencilAttachment)
		{
			imageFormat = Utils::FindSupportedFormat({ VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
			Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
				VulkanContext::Get()->GetDevice(),
				m_Specification.Width,
				m_Specification.Height,
				imageFormat,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				VK_SAMPLE_COUNT_1_BIT,
				1,
				0,
				1,
				image,
				imageMemory);
			Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), image, imageFormat, VK_IMAGE_VIEW_TYPE_2D, 1, depthAspectFlag, 1, imageView);

			m_SwapChainAdditionalAttachments.push_back(imageView);

			m_DepthStencilImages.push_back(image);
			m_DepthStencilImagesMemory.push_back(imageMemory);

			attachments.push_back(imageView);
		}
		// create multi sampled attachments if needed
		if (m_Specification.Samples > 1)
		{
			// create color attachments
			for (int i = 0; i < spec.ColorAttachments.size(); i++)
			{
				imageFormat = renderpassSpec.ColorAttachmentsFormat.at(i);
				Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
					VulkanContext::Get()->GetDevice(),
					m_Specification.Width,
					m_Specification.Height,
					imageFormat,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
					VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
					Utils::SampleCountToVulkanFlag(m_Specification.Samples),
					1,
					0,
					1,
					image,
					imageMemory);
				Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), image, imageFormat, VK_IMAGE_VIEW_TYPE_2D, 1, VK_IMAGE_ASPECT_COLOR_BIT, 1, imageView);

				m_SwapChainAdditionalAttachments.push_back(imageView);

				m_ColorImages.push_back(image);
				m_ColorImagesMemory.push_back(imageMemory);

				attachments.push_back(imageView);
			}
			// create depth stencil attachments
			if (renderpassSpec.EnableDepthStencilAttachment)
			{
				imageFormat = Utils::FindSupportedFormat({ VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
				Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
					VulkanContext::Get()->GetDevice(),
					m_Specification.Width,
					m_Specification.Height,
					imageFormat,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
					VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
					Utils::SampleCountToVulkanFlag(m_Specification.Samples),
					1,
					0,
					1,
					image,
					imageMemory);
				Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), image, imageFormat, VK_IMAGE_VIEW_TYPE_2D, 1, depthAspectFlag, 1, imageView);

				m_SwapChainAdditionalAttachments.push_back(imageView);

				m_DepthStencilImages.push_back(image);
				m_DepthStencilImagesMemory.push_back(imageMemory);

				attachments.push_back(imageView);
			}
		}
	

		VkFramebufferCreateInfo         framebufferInfo{};
		framebufferInfo.sType			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass		= m_RenderPass->GetRenderPass();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments	= attachments.data();
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
			for(int i = 0; i < m_Attachments.size(); i++)
			{
				vkDestroyImageView(VulkanContext::Get()->GetDevice(), m_Attachments.at(i), nullptr);
			}
			for (int i = 0; i < m_SwapChainAdditionalAttachments.size(); i++)
			{
				vkDestroyImageView(VulkanContext::Get()->GetDevice(), m_SwapChainAdditionalAttachments.at(i), nullptr);
			}
			for(int i = 0; i < m_ColorImages.size(); i++)
			{
				vkDestroyImage(VulkanContext::Get()->GetDevice(), m_ColorImages.at(i), nullptr);
				vkFreeMemory(VulkanContext::Get()->GetDevice(), m_ColorImagesMemory.at(i), nullptr);
			}
			for(int i = 0; i < m_DepthStencilImages.size(); i++)
			{
				vkDestroyImage(VulkanContext::Get()->GetDevice(), m_DepthStencilImages.at(i), nullptr);
				vkFreeMemory(VulkanContext::Get()->GetDevice(), m_DepthStencilImagesMemory.at(i), nullptr);
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
		for (int i = 0; i < m_RenderTargets.size(); i++)
		{
			m_RenderTargets.at(i)->SetImageLayout(cmd, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
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
		if (m_DepthStencilImages.size() > 0)
		{
			clearValues.at(clearValues.size() - 1) = { Graphics::IsReverseDepth() ? 0 : 1.0f, 0 };
		}
		renderPassInfo.clearValueCount			= static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues				= clearValues.data();
		vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}
	void VulkanFrameBuffer::BeginPresentRender(CommandBuffer* cmdBuffer)
	{
		VkCommandBuffer cmd = ((VulkanCommandBuffer*)cmdBuffer)->GetCommandBuffer();
		for (int i = 0; i < m_RenderTargets.size(); i++)
		{
			m_RenderTargets.at(i)->SetImageLayout(cmd, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
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
		renderPassInfo.renderArea.extent.height	= m_Specification.Height;

		Vector4									setClearColor = VulkanContext::Get()->GetClearColor();
		VkClearValue							clearColor = {};
		clearColor.color = { { setClearColor.r, setClearColor.g, setClearColor.b, setClearColor.a} };

		std::vector<VkClearValue>				clearValues(VulkanContext::Get()->GetSwapChainImageCount(), clearColor);
		if (m_DepthStencilImages.size() > 0)
		{
			clearValues.push_back({ Graphics::IsReverseDepth() ? 0 : 1.0f, 0 });
		}
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();
		vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}
	void VulkanFrameBuffer::EndPresentRender(CommandBuffer* cmdBuffer)
	{
		VkCommandBuffer cmd = ((VulkanCommandBuffer*)cmdBuffer)->GetCommandBuffer();
		vkCmdEndRenderPass(cmd);

		for (int i = 0; i < m_RenderTargets.size(); i++)
		{
			m_RenderTargets.at(i)->SetImageLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		if (m_DepthStencil)
		{
			m_DepthStencil->SetImageLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	}
	void VulkanFrameBuffer::End(CommandBuffer* cmdBuffer)
	{
		VkCommandBuffer cmd = ((VulkanCommandBuffer*)cmdBuffer)->GetCommandBuffer();
		vkCmdEndRenderPass(cmd);

		for (int i = 0; i < m_RenderTargets.size(); i++)
		{
			m_RenderTargets.at(i)->SetImageLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		if (m_DepthStencil)
		{
			m_DepthStencil->SetImageLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	}
	Ref<Texture2D> VulkanFrameBuffer::GetRenderTarget(int index)
	{
		GE_CORE_ASSERT(index < m_RenderTargets.size(), "index out of range");
		Ref<VulkanTexture2D> texture = m_RenderTargets.at(index);
		return texture;
	}
	Ref<Texture2D> VulkanFrameBuffer::GetDepthStencil()
	{
		GE_CORE_ASSERT(m_DepthStencilImages.size() > 0, "no depth frame buffer");
		Ref<VulkanTexture2D> texture = m_DepthStencil;
		return texture;
	}
	Ref<VulkanFrameBuffer> VulkanFrameBuffer::CreateForSwapChain(const Ref<VulkanRenderPass>& renderPass, const FrameBufferSpecificationForVulkan spec, const RenderPassSpecificationForVulkan& renderpassSpec)
	{
		return CreateRef<VulkanFrameBuffer>(renderPass, spec, renderpassSpec);
	}
	void VulkanFrameBuffer::CreateBuffer()
	{
		// if multi samples, we need to create a temporary image for each color attachment and depth attachment
		// the original images will be placed in the end of the attachments vector
		// the resolved images will be placed in the beginning of the attachments vector
		// and get render target will return the resolved images
		m_RenderTargets.clear();

		m_Attachments.clear();

		m_ColorImages.clear();
		m_ColorImagesMemory.clear();

		m_DepthStencilImages.clear();
		m_DepthStencilImagesMemory.clear();

		VkImage						image;
		VkImageView					imageView;
		VkDeviceMemory				imageMemory;
		VkFormat 					imageFormat;
		VkFlags						depthAspectFlag;
		// create color attachments
		for (int i = 0; i < m_Specification.RenderTargets.size(); i++)
		{
			imageFormat = Utils::FrameBufferTextureFormatToVulkanFormat(m_Specification.RenderTargets.at(i));
			Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
				VulkanContext::Get()->GetDevice(),
				m_Specification.Width,
				m_Specification.Height,
				imageFormat,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				VK_SAMPLE_COUNT_1_BIT,
				1,
				0,
				1,
				image,
				imageMemory);
			Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), image, imageFormat, VK_IMAGE_VIEW_TYPE_2D, 1, VK_IMAGE_ASPECT_COLOR_BIT, 1, imageView);

			m_Attachments.push_back(imageView);

			m_ColorImages.push_back(image);
			m_ColorImagesMemory.push_back(imageMemory);

			Ref<VulkanTexture2D> texture = CreateRef<VulkanTexture2D>(imageFormat, image, imageView, imageMemory, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
			m_RenderTargets.push_back(texture);
		}
		// create depth attachment
		if (m_Specification.DepthStencil != FRAME_BUFFER_TEXTURE_FORMAT_NONE)
		{
			switch (m_Specification.DepthStencil)
			{
			case FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8:
			{
				imageFormat = VK_FORMAT_D24_UNORM_S8_UINT;
				depthAspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
				break;
			}
			case FRAME_BUFFER_TEXTURE_FORMAT_DEPTH:
			{
				imageFormat = VK_FORMAT_D32_SFLOAT;
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
				imageFormat,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				VK_SAMPLE_COUNT_1_BIT,
				1,
				0,
				1,
				image,
				imageMemory);
			Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), image, imageFormat, VK_IMAGE_VIEW_TYPE_2D, 1, depthAspectFlag, 1, imageView);

			m_Attachments.push_back(imageView);

			m_DepthStencilImages.push_back(image);
			m_DepthStencilImagesMemory.push_back(imageMemory);

			m_DepthStencil = CreateRef<VulkanTexture2D>(imageFormat, image, imageView, imageMemory, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, depthAspectFlag);
		}
		// multi samples
		// create multi sampled attachments
		if (m_Specification.Samples > 1)
		{
			// create multi sample color attachments
			for (int i = 0; i < m_Specification.RenderTargets.size(); i++)
			{
				Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
					VulkanContext::Get()->GetDevice(),
					m_Specification.Width,
					m_Specification.Height,
					imageFormat,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					Utils::SampleCountToVulkanFlag(m_Specification.Samples),
					1,
					0,
					1,
					image,
					imageMemory);
				Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), image, imageFormat, VK_IMAGE_VIEW_TYPE_2D, 1, VK_IMAGE_ASPECT_COLOR_BIT, 1, imageView);

				m_Attachments.push_back(imageView);

				m_ColorImages.push_back(image);
				m_ColorImagesMemory.push_back(imageMemory);
			}
			// create multi sample depth attachment
			if (m_Specification.DepthStencil != FRAME_BUFFER_TEXTURE_FORMAT_NONE)
			{
				switch (m_Specification.DepthStencil)
				{
				case FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8:
				{
					imageFormat = VK_FORMAT_D24_UNORM_S8_UINT;
					depthAspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
					break;
				}
				case FRAME_BUFFER_TEXTURE_FORMAT_DEPTH:
				{
					imageFormat = VK_FORMAT_D32_SFLOAT;
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
					imageFormat,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					Utils::SampleCountToVulkanFlag(m_Specification.Samples),
					1,
					0,
					1,
					image,
					imageMemory);
				Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), image, imageFormat, VK_IMAGE_VIEW_TYPE_2D, 1, depthAspectFlag, 1, imageView);

				m_Attachments.push_back(imageView);

				m_DepthStencilImages.push_back(image);
				m_DepthStencilImagesMemory.push_back(imageMemory);
			}
		}

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