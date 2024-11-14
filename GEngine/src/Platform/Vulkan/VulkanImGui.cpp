#include "GEpch.h"
#include "VulkanImGui.h"
#include "GEngine/Application.h"
#include "ImGui/backends/imgui_impl_glfw.h"
#include "ImGui/backends/imgui_impl_vulkan.h"
#include "ImGui/backends/imgui_impl_vulkan.cpp"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "GEngine/Renderer/RenderCommand.h"

namespace GEngine {
	VkRenderPass				s_RenderPass;
	VkFramebuffer				s_FrameBuffer;
	VkImage						s_ColorImage;
	VkImageView					s_ColorImageView;
	VkDeviceMemory				s_ColorImageMemory;
	Vector2						s_Spec;
	Ref<VulkanTexture2D>		s_ImGuiImage;

	void VulkanImGui::OnAttach(GLFWwindow* window)
	{
		s_Spec.x				= Application::Get().GetWindow().GetWidth();
		s_Spec.y				= Application::Get().GetWindow().GetHeight();

		std::vector<VkAttachmentDescription>	attachments;
		VkAttachmentDescription					des = Utils::CreateAttachmentDescription(FrameBufferTextureFormat::RGBA8);
		des.format								= VK_FORMAT_R8G8B8A8_UNORM;
		attachments.push_back(des);

		std::vector<VkAttachmentReference>	colorAttachmentRefs;
		VkAttachmentReference				ref = Utils::CreateAttachmentReference(FrameBufferTextureFormat::RGBA8, 0);
		colorAttachmentRefs.push_back(ref);

		VkSubpassDescription			 subpass{};
		subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount	= static_cast<uint32_t>(colorAttachmentRefs.size());
		subpass.pColorAttachments		= colorAttachmentRefs.data();
		subpass.pDepthStencilAttachment = nullptr;


		VkSubpassDependency			 dependency{};
		dependency.srcSubpass		= VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass		= 0;
		dependency.srcStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask	= 0;
		dependency.dstStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask	= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo          renderPassInfo{};
		renderPassInfo.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount	= static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments		= attachments.data();
		renderPassInfo.subpassCount		= 1;
		renderPassInfo.pSubpasses		= &subpass;
		renderPassInfo.dependencyCount	= 1;
		renderPassInfo.pDependencies	= &dependency;

		VK_CHECK_RESULT(vkCreateRenderPass(VulkanContext::Get()->GetDevice(), &renderPassInfo, nullptr, &s_RenderPass));

		CreateBuffer();

		ImGui_ImplGlfw_InitForVulkan(window, true);
		VkDescriptorPool descriptorPool;

		std::vector<VkDescriptorPoolSize> poolSizes =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER,					1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,				1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,				1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,		1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,		1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,			1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,			1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,	1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,	1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,			1000 }
		};


		VkDescriptorPoolCreateInfo	poolInfo{};
		poolInfo.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount		= static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes			= poolSizes.data();
		poolInfo.maxSets			= 100 * poolSizes.size();

		VK_CHECK_RESULT(vkCreateDescriptorPool(VulkanContext::Get()->GetDevice(), &poolInfo, nullptr, &descriptorPool));

		ImGui_ImplVulkan_InitInfo		info{};
		info.Instance					= VulkanContext::Get()->GetInstance();
		info.PhysicalDevice				= VulkanContext::Get()->GetPhysicalDevice();
		info.Device						= VulkanContext::Get()->GetDevice();
		info.QueueFamily				= VulkanContext::Get()->GetQueueFamily().GraphicsFamily.value();
		info.Queue						= VulkanContext::Get()->GetGraphicsQueue();
		info.PipelineCache				= nullptr;
		info.MinImageCount				= 2;
		info.ImageCount					= VulkanContext::Get()->GetSwapChainImage().size();
		info.DescriptorPool				= descriptorPool;
		info.Subpass					= 0;
		info.Allocator					= nullptr;
		info.CheckVkResultFn			= nullptr;
		ImGui_ImplVulkan_Init(&info, s_RenderPass);

		VkCommandBuffer CmdBuffer		= VulkanContext::Get()->BeginSingleTimeCommands();
		ImGui_ImplVulkan_CreateFontsTexture(CmdBuffer);
		VulkanContext::Get()->EndSingleTimeCommands(CmdBuffer);
		vkDeviceWaitIdle(VulkanContext::Get()->GetDevice());
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	void VulkanImGui::OnDetach()
	{
		ImGui_ImplVulkan_Shutdown();
	}

	void VulkanImGui::Begin()
	{
		if ((s_Spec.x != Application::Get().GetWindow().GetWidth() ||
			s_Spec.y != Application::Get().GetWindow().GetHeight()) &&
			(Application::Get().GetWindow().GetWidth() != 0 &&
				Application::Get().GetWindow().GetHeight() != 0))
		{
			s_Spec.x = Application::Get().GetWindow().GetWidth();
			s_Spec.y = Application::Get().GetWindow().GetHeight();
			vkDestroyFramebuffer(VulkanContext::Get()->GetDevice(), s_FrameBuffer, nullptr);
			CreateBuffer();
		}
		ImGui_ImplVulkan_NewFrame();
	}

	void VulkanImGui::End()
	{
		s_ImGuiImage->SetImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		RenderCommand::BeginDrawCommand();
		VkRenderPassBeginInfo					renderPassInfo{};
		renderPassInfo.sType					= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass				= s_RenderPass;
		renderPassInfo.framebuffer				= s_FrameBuffer;
		renderPassInfo.renderArea.offset		= { 0, 0 };
		renderPassInfo.renderArea.extent.width	= s_Spec.x;
		renderPassInfo.renderArea.extent.height = s_Spec.y;

		VkClearValue							clearColor = {};
		clearColor.color						= { { 0.0f, 0.0f, 0.0f, 0.0f} };

		renderPassInfo.clearValueCount			= 1;
		renderPassInfo.pClearValues				= &clearColor;
		vkCmdBeginRenderPass(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), VulkanContext::Get()->GetCurrentDrawCommandBuffer());

		vkCmdEndRenderPass(VulkanContext::Get()->GetCurrentDrawCommandBuffer());

		RenderCommand::EndDrawCommand();
	}

	Ref<Texture2D> VulkanImGui::GetImGuiTexture()
	{
		return s_ImGuiImage;
	}

	void VulkanImGui::CreateBuffer()
	{
		Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
			VulkanContext::Get()->GetDevice(),
			s_Spec.x,
			s_Spec.y,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			s_ColorImage,
			s_ColorImageMemory);
		Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), s_ColorImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, s_ColorImageView);

		VkFramebufferCreateInfo			framebufferInfo{};
		framebufferInfo.sType			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass		= s_RenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments	= &s_ColorImageView;
		framebufferInfo.width			= s_Spec.x;
		framebufferInfo.height			= s_Spec.y;
		framebufferInfo.layers			= 1;

		VK_CHECK_RESULT(vkCreateFramebuffer(VulkanContext::Get()->GetDevice(), &framebufferInfo, nullptr, &s_FrameBuffer));

		s_ImGuiImage = CreateRef<VulkanTexture2D>(VK_FORMAT_R8G8B8A8_UNORM, s_ColorImage, s_ColorImageView, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, true);
	}

}
