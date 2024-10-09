#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/GraphicsContext.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"
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

		virtual void Init(const unsigned int width, const unsigned int height) override;
		virtual void Uninit() override;
		virtual void SwapBuffers() override;

		virtual void SetRequiredExtensions(std::vector<const char*> extensions) override { m_Extensions = extensions; }


		static VkDevice GetDevice() { return s_Device; }
		static VkExtent2D GetSwapChainExtent() { return s_SwapChainExtent; }
		static std::vector<VkImage> GetSwapChainImage() { return s_SwapChainImages; }
		static VkPhysicalDevice GetPhysicalDevice() { return s_PhysicalDevice; }
		static VkCommandBuffer GetCurrentCommandBuffer() { GE_CORE_ASSERT(s_PushedCommandBufferIndexs.size() > 0, "There are not commandbuffer be using!"); return s_CommandBuffer.GetCommandBuffer(s_PushedCommandBufferIndexs[s_PushedCommandBufferIndexs.size() - 1]); }
		static void PushCommandBuffer();
		static VkCommandBuffer PopCommandBuffer();
		static Vector4 GetClearColor() { return s_ClearColor; }
	private:
		void CreateInstance();
		bool CheckValidationLayerSupport();
		void SetupDebugMessenger();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void SetPhysicalDevice();
		bool IsDeviceSuitable(VkPhysicalDevice device);
		void CreateLogicalDevice();
		void CreateSurface();
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const unsigned int width, const unsigned int height);
		void CreateSwapChain(const unsigned int width, const unsigned int height);
		void CreateImageViews();
		void CreateCommandBuffers();
	private:
		GLFWwindow*							m_WindowHandle;

		std::vector<const char*>			m_Extensions;
		VkInstance							m_Instance;
		const std::vector<const char*>		m_ValidationLayers =
		{ "VK_LAYER_KHRONOS_validation"
		};
		VkDebugUtilsMessengerEXT			m_DebugMessenger;
		static VkPhysicalDevice				s_PhysicalDevice;
		static VkDevice						s_Device;
		VkQueue								m_GraphicsQueue;
		VkQueue								m_PresentQueue;
		VkSurfaceKHR						m_Surface;
		const std::vector<const char*>		m_DeviceExtensions =
		{ VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		  "VK_EXT_extended_dynamic_state3"
		};
		VkSwapchainKHR						m_SwapChain;
		static std::vector<VkImage>			s_SwapChainImages;
		VkFormat							m_SwapChainImageFormat;
		static VkExtent2D					s_SwapChainExtent;
		std::vector<VkImageView>			m_SwapChainImageViews;
		static VulkanCommandBuffer			s_CommandBuffer;
		static std::vector<int>				s_PushedCommandBufferIndexs;
		static Vector4						s_ClearColor;
	};
}


