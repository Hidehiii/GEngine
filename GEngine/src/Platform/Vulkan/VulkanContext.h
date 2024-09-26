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
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool IsComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	class VulkanContext : public GraphicsContext
	{
		public:
		VulkanContext(GLFWwindow* windowHandle);
		virtual ~VulkanContext() override;

		virtual void Init() override;
		virtual void Uninit() override;
		virtual void SwapBuffers() override;

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
	private:
		GLFWwindow*							m_WindowHandle;

		VkInstance							m_Instance;
		const std::vector<const char*>		m_ValidationLayers =
		{ "VK_LAYER_KHRONOS_validation" };
		VkDebugUtilsMessengerEXT			m_DebugMessenger;
		VkPhysicalDevice					m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice							m_Device;
		VkQueue								m_GraphicsQueue;
		VkQueue								m_PresentQueue;
		VkSurfaceKHR						m_Surface;
	};
}


