#pragma once
#include "GEngine/ImGui/PlatformImGui.h"
#include <vulkan/vulkan.h>


class GLFWwindow;
namespace GEngine
{
	
	class VulkanTexture2D;
	class VulkanCommandBuffer;

	class GENGINE_API VulkanImGui : public PlatformImGui
	{
	public:
		VulkanImGui() {}
		virtual void OnAttach(GLFWwindow* window) override;
		virtual void OnDetach() override;

		virtual void Begin() override;
		virtual void End() override;

		virtual Ref<Texture2D> GetImGuiTexture() override;
	private:
		void CreateBuffer();
		void CreateCommandBufferAndSyncObjects();
	private:
		VkRenderPass				m_RenderPass;
		VkFramebuffer				m_FrameBuffer;
		VkImage						m_ColorImage;
		VkImageView					m_ColorImageView;
		VkDeviceMemory				m_ColorImageMemory;
		Vector2						m_Spec;
		Ref<VulkanTexture2D>		m_ImGuiImage;
		VulkanCommandBuffer*		m_CommandBuffer;
		VkSemaphore					m_Semaphore;
		VkFence						m_Fence;
	};

}

