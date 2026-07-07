#include "GEpch.h"
#include "VulkanImGui.h"
#include "GEngine/Application.h"
#include "ImGui/backends/imgui_impl_glfw.h"
#include "ImGui/backends/imgui_impl_win32.h"
#include "ImGui/backends/imgui_impl_vulkan.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"

namespace GEngine {
	
	static RenderPassSpecification						s_Spec;	
	static Ref<VulkanRenderPass>						s_RenderPass = nullptr;
	static Ref<VulkanFrameBuffer>						s_FrameBuffer = nullptr;
	static std::vector<Ref<VulkanCommandBuffer>>		s_CommandBuffers;
	VulkanImGui::~VulkanImGui()
	{
		ImGui_ImplVulkan_Shutdown();
	}
	void VulkanImGui::OnAttach(void* window)
	{
		s_Spec.RenderTargets				= { FRAME_BUFFER_TEXTURE_FORMAT_RGBA8 };
		s_Spec.DepthStencil					= FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8;
		s_Spec.Samples						= 1;
		s_Spec.Operation.ColorBegin			= RENDER_PASS_BEGINE_OP_CLEAR;
		s_Spec.Operation.ColorEnd			= RENDER_PASS_END_OP_STORE;
		s_Spec.Operation.DepthStencilBegin	= RENDER_PASS_BEGINE_OP_CLEAR;
		s_Spec.Operation.DepthStencilEnd	= RENDER_PASS_END_OP_STORE;

		s_RenderPass = CreateRef<VulkanRenderPass>(s_Spec);

		s_FrameBuffer = CreateRef<VulkanFrameBuffer>(s_RenderPass, Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());

		CreateCommandBuffers();
		switch (Application::Get().GetConfig()->GetWindowManagerAPI())
		{
		case Config::CONFIG_WINDOW_MANAGER_API_GLFW:
			ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)window, true);
			break;
		case Config::CONFIG_WINDOW_MANAGER_API_WIN32:
			ImGui_ImplWin32_Init(window);
			break;
		default:
			GE_CORE_ASSERT(false, "Unknown window manager api");
			break;
		}
		
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
		poolInfo.flags				= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		VK_CHECK_RESULT(vkCreateDescriptorPool(VulkanContext::Get()->GetDevice(), &poolInfo, nullptr, &descriptorPool));

		ImGui_ImplVulkan_InitInfo		info{};
		info.Instance					= VulkanContext::Get()->GetInstance();
		info.PhysicalDevice				= VulkanContext::Get()->GetPhysicalDevice();
		info.Device						= VulkanContext::Get()->GetDevice();
		info.QueueFamily				= VulkanContext::Get()->GetQueueFamily().GraphicsFamily.value();
		info.Queue						= VulkanContext::Get()->GetGraphicsQueue();
		info.PipelineCache				= nullptr;
		info.MinImageCount				= Graphics::GetFrameCount();
		info.ImageCount					= VulkanContext::Get()->GetSwapChainImage().size();
		info.DescriptorPool				= descriptorPool;
		info.Subpass					= 0;
		info.Allocator					= nullptr;
		info.CheckVkResultFn			= nullptr;
		ImGui_ImplVulkan_Init(&info, s_RenderPass->GetRenderPass());

		VkCommandBuffer CmdBuffer		= VulkanContext::Get()->BeginSingleTimeGraphicsCommand();
		ImGui_ImplVulkan_CreateFontsTexture(CmdBuffer);
		VulkanContext::Get()->EndSingleTimeGraphicsCommand(CmdBuffer);
		vkDeviceWaitIdle(VulkanContext::Get()->GetDevice());
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	void VulkanImGui::Begin()
	{
		if ((s_FrameBuffer->GetWidth() != Application::Get().GetWindow().GetWidth() ||
			s_FrameBuffer->GetHeight() != Application::Get().GetWindow().GetHeight()) &&
			(Application::Get().GetWindow().GetWidth() != 0 &&
				Application::Get().GetWindow().GetHeight() != 0))
		{
			s_FrameBuffer = CreateRef<VulkanFrameBuffer>(s_RenderPass, Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
		}
		ImGui_ImplVulkan_NewFrame();
	}

	void VulkanImGui::End()
	{

		VkCommandBuffer cmd = s_CommandBuffers.at(Graphics::GetFrame())->GetCommandBuffer();

		VkCommandBufferBeginInfo    beginInfo{};
		beginInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags				= 0; // Optional
		beginInfo.pInheritanceInfo	= nullptr; // Optional

		vkResetCommandBuffer(cmd, 0);
		VK_CHECK_RESULT(vkBeginCommandBuffer(cmd, &beginInfo));

		s_FrameBuffer->Begin(s_CommandBuffers.at(Graphics::GetFrame()).get());

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

		s_FrameBuffer->End(s_CommandBuffers.at(Graphics::GetFrame()).get());

		VK_CHECK_RESULT(vkEndCommandBuffer(cmd));

		const std::vector<VkSemaphore> waitSemaphores = s_CommandBuffers.at(Graphics::GetFrame())->GetWaitSemaphores();
		const std::vector<VkPipelineStageFlags> waitStages(waitSemaphores.size(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		const std::vector<VkSemaphore> signalSemaphores = s_CommandBuffers.at(Graphics::GetFrame())->GetSignalSemaphores();

		VkSubmitInfo                    submitInfo{};
		submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount	= 1;
		submitInfo.pCommandBuffers		= &cmd;
		submitInfo.waitSemaphoreCount	= waitSemaphores.size();
		submitInfo.pWaitSemaphores		= waitSemaphores.data();
		submitInfo.pWaitDstStageMask	= waitStages.data();
		submitInfo.signalSemaphoreCount = signalSemaphores.size();
		submitInfo.pSignalSemaphores	= signalSemaphores.data();

		VK_CHECK_RESULT(vkQueueSubmit(VulkanContext::Get()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));

		s_CommandBuffers.at(Graphics::GetFrame())->ClearWaitSemaphores();
		s_CommandBuffers.at(Graphics::GetFrame())->ClearSignalSemaphores();
	}

	Ref<Texture2D> VulkanImGui::GetImGuiTexture()
	{
		return s_FrameBuffer->GetRenderTarget(0);
	}

	Ref<CommandBuffer> VulkanImGui::GetCommandBuffer()
	{
		return s_CommandBuffers.at(Graphics::GetFrame());
	}

	void VulkanImGui::CreateCommandBuffers()
	{
		s_CommandBuffers.resize(Graphics::GetFrameCount());

		std::vector<VkCommandBuffer>	cmds;
		cmds.resize(Graphics::GetFrameCount());

		VkCommandBufferAllocateInfo		allocInfo{};
		allocInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool			= VulkanContext::Get()->GetGraphicsCommandPool();
		allocInfo.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = Graphics::GetFrameCount();
		VK_CHECK_RESULT(vkAllocateCommandBuffers(VulkanContext::Get()->GetDevice(), &allocInfo, cmds.data()));

		for (int i = 0; i < s_CommandBuffers.size(); i++)
		{
			s_CommandBuffers.at(i) = VulkanCommandBuffer::Create(cmds.at(i), COMMAND_BUFFER_TYPE_GRAPHICS);
		}
	}

}
