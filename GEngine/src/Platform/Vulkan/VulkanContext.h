#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/GraphicsContext.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanDescriptor.h"
#include "Platform/Vulkan/VulkanFrameBuffer.h"
#include "GEngine/Math/Math.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>


namespace GEngine
{

	struct SwapChainSupportDetails 
	{
		VkSurfaceCapabilitiesKHR		Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR>	PresentModes;
	};

	class GENGINE_API VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext(GLFWwindow* windowHandle);
		virtual ~VulkanContext() override;

		virtual void				Init(const unsigned int width, const unsigned int height) override;
		virtual void				Uninit() override;
		virtual void				SwapBuffers() override;

		virtual void				SetRequiredExtensions(std::vector<const char*> extensions) override { m_Extensions = extensions; }
		virtual void				RecreateSwapChain(unsigned int width, unsigned int height) override;

		static VulkanContext*		Get() { return s_ContextInstance; }
		VkDevice					GetDevice() { return m_Device; }
		VkExtent2D					GetSwapChainExtent() { return m_SwapChainExtent; }
		std::vector<VkImage>		GetSwapChainImage() { return m_SwapChainImages; }
		VkPhysicalDevice			GetPhysicalDevice() { return m_PhysicalDevice; }
		void						BeginDrawCommandBuffer();
		VkCommandBuffer				EndDrawCommandBuffer();
		VkCommandBuffer				GetCurrentDrawCommandBuffer() { return m_CommandBuffer.GetCommandBuffer(m_DrawUsedCommandBufferIndex); }
		void						SetClearColor(Vector4 color) { m_ClearColor = color; }
		Vector4						GetClearColor() { return m_ClearColor; }
		VkInstance					GetInstance() { return m_Instance; }
		VkDescriptorPool			GetDescriptorPool() { return m_Descriptor.GetDescriptorPool(); }
		VkCommandBuffer				BeginSingleTimeCommands();
		void						EndSingleTimeCommands(VkCommandBuffer commandBuffer);
		QueueFamilyIndices			GetQueueFamily() { return m_QueueFamily; }
		VkQueue						GetGraphicsQueue() { return m_GraphicsQueue; }
		VkQueue						GetPresentQueue() { return m_PresentQueue; }

		VkSwapchainKHR				GetSwapChain() { return m_SwapChain; }
		Ref<VulkanFrameBuffer>		GetFrameBuffer(int index) { return m_SwapChainFrameBuffers[index % m_SwapChainFrameBuffers.size()]; }
		VkFence&					GetCurrentFence() { return m_Fences.at(m_FenceIndex); }
		VkSemaphore&				GetCurrentSemaphore() { return m_Semaphores.at(m_SemaphoreIndex); }
		void                        MoveToNextSemaphore() { m_SemaphoreIndex = (m_SemaphoreIndex + 1) % m_Semaphores.size(); }
		
	private:
		void						CreateInstance();
		bool						CheckValidationLayerSupport();
		void						SetupDebugMessenger();
		void						PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void						SetPhysicalDevice();
		bool						IsDeviceSuitable(VkPhysicalDevice device);
		void						CreateLogicalDevice();
		void						CreateSurface();
		bool						CheckDeviceExtensionSupport(VkPhysicalDevice device);
		void						CheckInstanceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails		QuerySwapChainSupport(VkPhysicalDevice device);
		QueueFamilyIndices			FindQueueFamilies(VkPhysicalDevice device);
		VkSurfaceFormatKHR			ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR			ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D					ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const unsigned int width, const unsigned int height);
		void						CreateSwapChain(const unsigned int width, const unsigned int height);
		void						CreateImageViews();
		void						CreateFrameBuffer();
		void						CreateCommandBuffers();
		void						CreateDescriptor();
		void						CreateSyncObjects();
		void						CleanUpSwapChain();
	private:
		GLFWwindow*							m_WindowHandle;
		static VulkanContext*				s_ContextInstance;
		std::vector<const char*>			m_Extensions;
		VkInstance							m_Instance;
		const std::vector<const char*>		m_ValidationLayers =
		{ 
			"VK_LAYER_KHRONOS_validation"
		};
		VkDebugUtilsMessengerEXT			m_DebugMessenger;
		VkPhysicalDevice					m_PhysicalDevice;
		VkDevice							m_Device;
		VkQueue								m_GraphicsQueue;
		VkQueue								m_PresentQueue;
		VkSurfaceKHR						m_Surface;
		std::vector<const char*>			m_DeviceExtensions =
		{ 
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_MAINTENANCE1_EXTENSION_NAME
		};
		std::vector<const char*>			m_InstanceExtensions =
		{
			VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
			VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME,
			VK_EXT_IMAGE_COMPRESSION_CONTROL_EXTENSION_NAME,
			VK_EXT_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME,
			VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
		};
		VkSwapchainKHR						m_SwapChain;
		std::vector<VkImage>				m_SwapChainImages;
		VkFormat							m_SwapChainImageFormat;
		VkExtent2D							m_SwapChainExtent;
		std::vector<VkImageView>			m_SwapChainImageViews;
		std::vector<Ref<VulkanFrameBuffer>>	m_SwapChainFrameBuffers;
		VulkanCommandBuffer					m_CommandBuffer;
		int									m_DrawUsedCommandBufferIndex;
		Vector4								m_ClearColor = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		VulkanDescriptor					m_Descriptor;
		QueueFamilyIndices					m_QueueFamily;
		std::vector<VkSemaphore>			m_Semaphores;
		int                                 m_SemaphoreIndex = 0;
		std::vector<VkFence>				m_Fences;
		int                                 m_FenceIndex = 0;
	};
	
}


