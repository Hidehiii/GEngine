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
	static VkRenderPass	s_RenderPass;

	void VulkanImGui::OnAttach(GLFWwindow* window)
	{

		std::vector<VkAttachmentDescription>  attachments;
		VkAttachmentDescription		des = Utils::CreateAttachmentDescription(FrameBufferTextureFormat::RGBA8); 
			des.format = VK_FORMAT_B8G8R8A8_UNORM;
		attachments.push_back(des);

		std::vector<VkAttachmentReference> colorAttachmentRefs;
		VkAttachmentReference		ref = Utils::CreateAttachmentReference(FrameBufferTextureFormat::RGBA8, 0);
		colorAttachmentRefs.push_back(ref);

		VkSubpassDescription			 subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
		subpass.pColorAttachments = colorAttachmentRefs.data();
		subpass.pDepthStencilAttachment = nullptr;


		VkSubpassDependency			 dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo          renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VK_CHECK_RESULT(vkCreateRenderPass(VulkanContext::Get()->GetDevice(), &renderPassInfo, nullptr, &s_RenderPass));

		ImGui_ImplGlfw_InitForVulkan(window, true);
		VkDescriptorPool descriptorPool;

		std::vector<VkDescriptorPoolSize> poolSizes =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER,					100 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	100 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,				100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,				100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,		100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,		100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,			100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,			100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,	100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,	100 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,			100 }
		};


		VkDescriptorPoolCreateInfo	poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 100;

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

		VkCommandBuffer CmdBuffer = VulkanContext::Get()->BeginSingleTimeCommands();
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
		ImGui_ImplVulkan_NewFrame();
	}

	void VulkanImGui::End()
	{
		RenderCommand::BeginDrawCommand();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), VulkanContext::Get()->GetCurrentDrawCommandBuffer());
		RenderCommand::EndDrawCommand();
	}

}
