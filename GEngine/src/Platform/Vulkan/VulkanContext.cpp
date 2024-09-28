#include "GEpch.h"
#include "VulkanContext.h"

#include <set>

namespace GEngine
{
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            GE_CORE_ERROR("Validation layer: {0}", pCallbackData->pMessage);
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            GE_CORE_WARN("Validation layer: {0}", pCallbackData->pMessage);
        }
        return VK_FALSE;
    }
    static VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto    func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    static void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator)
    {
        auto    func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }

	VulkanContext::VulkanContext(GLFWwindow* windowHandle)
        : m_WindowHandle(windowHandle)
	{
		m_Surface = nullptr;
	}
	VulkanContext::~VulkanContext()
	{
	}
	void VulkanContext::Init(const unsigned int width, const unsigned int height)
	{
        CreateInstance();
        SetupDebugMessenger();
        CreateSurface();
        SetPhysicalDevice();
        CreateLogicalDevice();
        CreateSwapChain(width, height);
        CreateImageViews();
	}
    void VulkanContext::Uninit()
    {
        for (auto imageView : m_SwapChainImageViews)
		{
			vkDestroyImageView(m_Device, imageView, nullptr);
		}
        vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);
#ifdef GE_DEBUG
        DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
#endif
        vkDestroyDevice(m_Device, nullptr);
        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        vkDestroyInstance(m_Instance, nullptr);
    }
	void VulkanContext::SwapBuffers()
	{

	}
    void VulkanContext::CreateInstance()
    {
#ifdef GE_DEBUG
        if (CheckValidationLayerSupport() == false)
        {
            GE_CORE_ERROR("Validation layers requested, but not available!");
        }
#endif
        VkApplicationInfo                       appInfo = {};
        appInfo.sType                           = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName                = "GEngine";
        appInfo.applicationVersion              = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName                     = "GEngine";
        appInfo.engineVersion                   = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion                      = VK_API_VERSION_1_0;

        VkInstanceCreateInfo                    createInfo = {};
        createInfo.sType                        = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo             = &appInfo;
        createInfo.enabledExtensionCount        = static_cast<uint32_t>(m_Extensions.size());
        createInfo.ppEnabledExtensionNames      = m_Extensions.data();
#ifdef GE_DEBUG
        createInfo.enabledLayerCount            = static_cast<uint32_t>(m_ValidationLayers.size());
        createInfo.ppEnabledLayerNames          = m_ValidationLayers.data();

        VkDebugUtilsMessengerCreateInfoEXT       debugCreateInfo;
        PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#else 
        createInfo.ppEnabledExtensionNames      = nullptr;
        createInfo.enabledLayerCount            = 0;
#endif

        if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
        {
            GE_CORE_ERROR("Failed to create Vulkan Instance!");
        }
    }
    bool VulkanContext::CheckValidationLayerSupport()
    {
        uint32_t                                layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties>          availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : m_ValidationLayers)
        {
            bool                                layerFound = false;

            for (const auto& layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
            {
                return false;
            }
        }
        return true;
    }
    void VulkanContext::SetupDebugMessenger()
    {
#ifndef GE_DEBUG
        return;
#endif
        VkDebugUtilsMessengerCreateInfoEXT       createInfo;
        PopulateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
        {
            GE_CORE_ERROR("failed to set up debug messenger!");
        }
    }
    void VulkanContext::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = {};
        createInfo.sType            = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity  = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType      = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                    | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback   = DebugCallback;
    }
    void VulkanContext::SetPhysicalDevice()
    {
        uint32_t                            deviceCount = 0;
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
        if (deviceCount == 0)
        {
            GE_CORE_ERROR("Failed to find GPUs with Vulkan support!");
        }
        std::vector<VkPhysicalDevice>       devices(deviceCount);
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());
        for (const auto& device : devices)
        {
            if (IsDeviceSuitable(device))
            {
                m_PhysicalDevice = device;
                break;
            }
        }
        if (m_PhysicalDevice == VK_NULL_HANDLE)
        {
            GE_CORE_ERROR("Failed to find a suitable GPU!");
        }
    }
    bool VulkanContext::IsDeviceSuitable(VkPhysicalDevice device)
    {
        QueueFamilyIndices          indices                 = FindQueueFamilies(device);

        bool                        extensionsSupported     = CheckDeviceExtensionSupport(device);
        bool                        swapChainAdequate       = false;
        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupportDetails = QuerySwapChainSupport(device);
            swapChainAdequate                               = !swapChainSupportDetails.Formats.empty() && !swapChainSupportDetails.PresentModes.empty();
        }
        return indices.IsComplete() && extensionsSupported && swapChainAdequate;
    }
    QueueFamilyIndices VulkanContext::FindQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices                  indices;
        uint32_t                            queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int                                 i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.GraphicsFamily = i;
            }
            VkBool32                        presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);
            if (presentSupport)
            {
                indices.PresentFamily = i;
            }
            if (indices.IsComplete())
            {
                break;
            }
            i++;
        }
        return indices;
    }
    void VulkanContext::CreateLogicalDevice()
    {
        QueueFamilyIndices                  indices = FindQueueFamilies(m_PhysicalDevice);
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t>                  uniqueQueueFamilies = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };
        float                               queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo         queueCreateInfo = {};
            queueCreateInfo.sType           = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount      = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }
        VkPhysicalDeviceFeatures            deviceFeatures = {};
        VkDeviceCreateInfo                  createInfo = {};
        createInfo.sType                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount     = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos        = queueCreateInfos.data();
        createInfo.pEnabledFeatures         = &deviceFeatures;
        createInfo.enabledExtensionCount    = static_cast<uint32_t>(m_DeviceExtensions.size());
        createInfo.ppEnabledExtensionNames  = m_DeviceExtensions.data();
#ifdef GE_DEBUG
        createInfo.enabledLayerCount        = static_cast<uint32_t>(m_ValidationLayers.size());
        createInfo.ppEnabledLayerNames      = m_ValidationLayers.data();
#else
        createInfo.enabledLayerCount        = 0;
#endif
        if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS)
        {
            GE_CORE_ERROR("Failed to create logical device!");
        }
        vkGetDeviceQueue(m_Device, indices.GraphicsFamily.value(), 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, indices.PresentFamily.value(), 0, &m_PresentQueue);
    }
    void VulkanContext::CreateSurface()
    {
        if (glfwCreateWindowSurface(m_Instance, m_WindowHandle, nullptr, &m_Surface) != VK_SUCCESS)
        {
			GE_CORE_ERROR("Failed to create window surface!");
		}
    }
    bool VulkanContext::CheckDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
    }
    SwapChainSupportDetails VulkanContext::QuerySwapChainSupport(VkPhysicalDevice device)
    {
        SwapChainSupportDetails             details;
        uint32_t                            formatCount;
        uint32_t                            presentModeCount;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.Capabilities);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);
        if (formatCount != 0)
        {
            details.Formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.Formats.data());
        }
        if (presentModeCount != 0)
        {
            details.PresentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, details.PresentModes.data());
        }
        return details;
    }
    VkSurfaceFormatKHR VulkanContext::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format          == VK_FORMAT_B8G8R8A8_SRGB 
                && availableFormat.colorSpace   == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
            {
				return availableFormat;
			}
		}
        return availableFormats[0];
    }
    VkPresentModeKHR VulkanContext::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
    }
    VkExtent2D VulkanContext::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const unsigned int width, const unsigned int height)
    {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
        {
			return capabilities.currentExtent;
		}
		VkExtent2D      actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};
		actualExtent.width      = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height     = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		return actualExtent;
    }
    void VulkanContext::CreateSwapChain(const unsigned int width, const unsigned int height)
    {
        SwapChainSupportDetails     swapChainSupportDetails = QuerySwapChainSupport(m_PhysicalDevice);

        VkSurfaceFormatKHR          surfaceFormat           = ChooseSwapSurfaceFormat(swapChainSupportDetails.Formats);
        VkPresentModeKHR            presentMode             = ChooseSwapPresentMode(swapChainSupportDetails.PresentModes);
        VkExtent2D                  extent                  = ChooseSwapExtent(swapChainSupportDetails.Capabilities, width, height);

        uint32_t                    imageCount              = swapChainSupportDetails.Capabilities.minImageCount + 1;
        if (swapChainSupportDetails.Capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupportDetails.Capabilities.maxImageCount)
        {
            imageCount                                      = swapChainSupportDetails.Capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR    createInfo{};
		createInfo.sType                                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface                                  = m_Surface;
		createInfo.minImageCount                            = imageCount;
		createInfo.imageFormat                              = surfaceFormat.format;
		createInfo.imageColorSpace                          = surfaceFormat.colorSpace;
		createInfo.imageExtent                              = extent;
		createInfo.imageArrayLayers                         = 1;
		createInfo.imageUsage                               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices          indices                 = FindQueueFamilies(m_PhysicalDevice);
		uint32_t                    queueFamilyIndices[]    = { 
                                                            indices.GraphicsFamily.value(), 
                                                            indices.PresentFamily.value() };

		if (indices.GraphicsFamily != indices.PresentFamily)
        {
			createInfo.imageSharingMode                     = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount                = 2;
			createInfo.pQueueFamilyIndices                  = queueFamilyIndices;
		}
		else 
        {
			createInfo.imageSharingMode                     = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount                = 0; // Optional
			createInfo.pQueueFamilyIndices                  = nullptr; // Optional
		}

        createInfo.preTransform                             = swapChainSupportDetails.Capabilities.currentTransform;
        createInfo.compositeAlpha                           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode                              = presentMode;
		createInfo.clipped                                  = VK_TRUE;
        createInfo.oldSwapchain                             = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
        {
            GE_CORE_ERROR("fail to create swap chain!");
        }

        vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, nullptr);
        m_SwapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, m_SwapChainImages.data());

        m_SwapChainImageFormat                              = surfaceFormat.format;
        m_SwapChainExtent                                   = extent;
    }
    void VulkanContext::CreateImageViews()
    {
        m_SwapChainImageViews.resize(m_SwapChainImages.size());
        for (size_t i = 0; i < m_SwapChainImages.size(); i++)
        {
            VkImageViewCreateInfo                           createInfo{};
            createInfo.sType                                = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image                                = m_SwapChainImages[i];
            createInfo.viewType                             = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format                               = m_SwapChainImageFormat;
            createInfo.components.r                         = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g                         = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b                         = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a                         = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask          = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel        = 0;
            createInfo.subresourceRange.levelCount          = 1;
            createInfo.subresourceRange.baseArrayLayer      = 0;
            createInfo.subresourceRange.layerCount          = 1;

            if (vkCreateImageView(m_Device, &createInfo, nullptr, &m_SwapChainImageViews[i]) != VK_SUCCESS)
            {
				GE_CORE_ERROR("failed to create image views!");
			}
        }
    }
}