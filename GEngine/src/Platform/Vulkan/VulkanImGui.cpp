#include "GEpch.h"
#include "VulkanImGui.h"
#include "GEngine/Application.h"
#include "ImGui/backends/imgui_impl_glfw.h"
#include "ImGui/backends/imgui_impl_vulkan.h"
#include "ImGui/backends/imgui_impl_vulkan.cpp"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "GEngine/Renderer/RenderCommand.h"
#include "Platform/Vulkan/VulkanFrameBuffer.h"

namespace GEngine {
	static Ref<FrameBuffer>	s_FrameBuffer = nullptr;


	void VulkanImGui::OnAttach(GLFWwindow* window)
	{
		FrameBufferSpecification fspec;
		fspec.Attachments = { FrameBufferTextureFormat::RGBA8, FrameBufferTextureFormat::DEPTH };
		fspec.Width = Application::Get().GetWindow().GetWidth();
		fspec.Height = Application::Get().GetWindow().GetHeight();
		s_FrameBuffer = FrameBuffer::Create(fspec);

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
		info.Allocator					= nullptr;
		info.CheckVkResultFn			= nullptr;
		ImGui_ImplVulkan_Init(&info, std::dynamic_pointer_cast<VulkanFrameBuffer>(s_FrameBuffer)->GetRenderPass());

		VkCommandBuffer CmdBuffer = VulkanContext::Get()->BeginSingleTimeCommands();
		ImGui_ImplVulkan_CreateFontsTexture(CmdBuffer);
		VulkanContext::Get()->EndSingleTimeCommands(CmdBuffer);
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
		s_FrameBuffer->Begin();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), VulkanContext::Get()->GetCurrentDrawCommandBuffer());
		s_FrameBuffer->End();
		RenderCommand::EndDrawCommand();
	}

}
