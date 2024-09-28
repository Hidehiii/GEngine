#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/GraphicsContext.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <optional>

namespace GEngine
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> GraphicsFamily;
		std::optional<uint32_t> PresentFamily;

		bool IsComplete()
		{
			return GraphicsFamily.has_value() && PresentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails 
	{
		VkSurfaceCapabilitiesKHR		Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR>	PresentModes;
	};

	class VulkanContext : public GraphicsContext
	{
		public:
		VulkanContext(GLFWwindow* windowHandle);
		virtual ~VulkanContext() override;

		virtual void Init(const unsigned int width, const unsigned int height) override;
		virtual void Uninit() override;
		virtual void SwapBuffers() override;

		virtual void SetRequiredExtensions(std::vector<const char*> extensions) override { m_Extensions = extensions; }
	private:
		void CreateInstance();
		bool CheckValidationLayerSupport();
		void SetupDebugMessenger();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void SetPhysicalDevice();
		bool IsDeviceSuitable(VkPhysicalDevice device);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		void CreateLogicalDevice();
		void CreateSurface();
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const unsigned int width, const unsigned int height);
		void CreateSwapChain(const unsigned int width, const unsigned int height);
		void CreateImageViews();
	private:
		GLFWwindow*							m_WindowHandle;

		std::vector<const char*>			m_Extensions;
		VkInstance							m_Instance;
		const std::vector<const char*>		m_ValidationLayers =
		{ "VK_LAYER_KHRONOS_validation"
		};
		VkDebugUtilsMessengerEXT			m_DebugMessenger;
		VkPhysicalDevice					m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice							m_Device;
		VkQueue								m_GraphicsQueue;
		VkQueue								m_PresentQueue;
		VkSurfaceKHR						m_Surface;
		const std::vector<const char*>		m_DeviceExtensions =
		{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		VkSwapchainKHR						m_SwapChain;
		std::vector<VkImage>				m_SwapChainImages;
		VkFormat							m_SwapChainImageFormat;
		VkExtent2D							m_SwapChainExtent;
		std::vector<VkImageView>			m_SwapChainImageViews;
	};
}


