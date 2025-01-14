#include "GEpch.h"
#include "VulkanContext.h"
#include "GEngine/Application.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "GEngine/Renderer/Renderer.h"
#include <set>

namespace GEngine
{
    VulkanContext* VulkanContext::s_ContextInstance = nullptr;

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
        s_ContextInstance = this;
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
        CreateCommandBuffers();
        CreateDescriptor();
        CreateFrameBuffer();
        CreateSyncObjects();
	}
    void VulkanContext::Uninit()
    {
        vkDeviceWaitIdle(m_Device);
        vkQueueWaitIdle(m_GraphicsQueue);
        for(int i = 0;i < m_Semaphores.size();i++)
        {
            vkDestroySemaphore(m_Device, m_Semaphores[i], nullptr);
		}
        for (int i = 0; i < m_Fences.size(); i++)
		{
			vkDestroyFence(m_Device, m_Fences[i], nullptr);
		}
        m_Descriptor.Release();
        m_CommandBuffer.Release();
        for (auto frameBuffer : m_SwapChainFrameBuffers)
        {
            vkDestroyFramebuffer(m_Device, frameBuffer->GetFrameBuffer(), nullptr);
        }
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

        m_Device = nullptr;
        m_Instance = nullptr;
    }
	void VulkanContext::SwapBuffers()
	{
        
	}
    VkFence& VulkanContext::GetCurrentFence()
    {
        return m_Fences.at(Renderer::GetCurrentFrame());
    }
    VkSemaphore& VulkanContext::GetCurrentSemaphore()
    {
        return m_Semaphores.at(m_SemaphoreIndexs.at(Renderer::GetCurrentFrame()) + Renderer::GetCurrentFrame() * m_SyncObjectSizePerFrame);
    }
    void VulkanContext::MoveToNextSemaphore()
    {
        m_SemaphoreIndexs.at(Renderer::GetCurrentFrame()) = (m_SemaphoreIndexs.at(Renderer::GetCurrentFrame()) + 1) % m_SyncObjectSizePerFrame;
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
        appInfo.apiVersion                      = VK_API_VERSION_1_3;

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
        createInfo.pNext                        = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#else 
        createInfo.ppEnabledLayerNames = nullptr;
        createInfo.enabledLayerCount            = 0;
#endif

        VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &m_Instance));
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

        VK_CHECK_RESULT(CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger));
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

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &deviceProperties);

        GE_CORE_INFO("          Device name     : {}", deviceProperties.deviceName);
        GE_CORE_INFO("          Driver version  : {}", deviceProperties.driverVersion);
        GE_CORE_INFO("          API version     : {}.{}.{}", VK_VERSION_MAJOR(deviceProperties.apiVersion), VK_VERSION_MINOR(deviceProperties.apiVersion), VK_VERSION_PATCH(deviceProperties.apiVersion));
        GE_CORE_INFO("          Vendor id       : {}", deviceProperties.vendorID);
        GE_CORE_INFO("          Device id       : {}", deviceProperties.deviceID);
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

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.IsComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy &&
            supportedFeatures.fragmentStoresAndAtomics && supportedFeatures.vertexPipelineStoresAndAtomics && supportedFeatures.geometryShader &&
            supportedFeatures.tessellationShader;
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
            if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))
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

        m_QueueFamily                               = FindQueueFamilies(m_PhysicalDevice);
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

        CheckInstanceExtensionSupport(m_PhysicalDevice);

        VkPhysicalDeviceFeatures2               deviceFeatures2 = {};
        deviceFeatures2.sType                   = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

        VkPhysicalDevice16BitStorageFeatures    bitsStorage16 = {};
        bitsStorage16.sType                     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES;
        deviceFeatures2.pNext                   = &bitsStorage16;

        VkPhysicalDeviceExtendedDynamicState2FeaturesEXT    extendedDynamicState2Features = {};
        extendedDynamicState2Features.sType                 = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT;
        bitsStorage16.pNext                                 = &extendedDynamicState2Features;
        
		VkPhysicalDeviceExtendedDynamicState3FeaturesEXT    extendedDynamicState3Features = {};
		extendedDynamicState3Features.sType                 = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT;
        extendedDynamicState2Features.pNext                 = &extendedDynamicState3Features;

        VkPhysicalDeviceSeparateDepthStencilLayoutsFeaturesKHR  separateDepthStencilLayoutFeaturesKHR = {};
        separateDepthStencilLayoutFeaturesKHR.sType             = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES_KHR;
        extendedDynamicState3Features.pNext                     = &separateDepthStencilLayoutFeaturesKHR;

        VkPhysicalDeviceImageCompressionControlFeaturesEXT  imageCompressionControlFeatrue = {};
        imageCompressionControlFeatrue.sType                = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_FEATURES_EXT;
        separateDepthStencilLayoutFeaturesKHR.pNext         = &imageCompressionControlFeatrue;

        VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT imageCompressionControlSwapchainFeatrue = {};
        imageCompressionControlSwapchainFeatrue.sType               = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_FEATURES_EXT;
        imageCompressionControlFeatrue.pNext                        = &imageCompressionControlSwapchainFeatrue;

        VkPhysicalDeviceShaderFloat16Int8Features       shaderFloat16Int8Feature = {};
        shaderFloat16Int8Feature.sType                  = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES;
        imageCompressionControlSwapchainFeatrue.pNext   = &shaderFloat16Int8Feature;

        

        vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &deviceFeatures2);

        VkDeviceCreateInfo                  createInfo = {};
        createInfo.sType                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount     = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos        = queueCreateInfos.data();
        createInfo.pNext                    = &deviceFeatures2;

        std::vector<const char*>            extensions = m_InstanceExtensions;
        for (const auto& extension : m_DeviceExtensions)
        {
            if(std::find(extensions.begin(), extensions.end(), extension) == extensions.end())
			    extensions.push_back(extension);
        }

        createInfo.enabledExtensionCount    = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames  = extensions.data();
#ifdef GE_DEBUG
        createInfo.enabledLayerCount        = static_cast<uint32_t>(m_ValidationLayers.size());
        createInfo.ppEnabledLayerNames      = m_ValidationLayers.data();
#else
        createInfo.enabledLayerCount        = 0;
        createInfo.ppEnabledLayerNames      = nullptr;
#endif
        VK_CHECK_RESULT(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device));

        LoadFunctionEXT(extensions);

		vkGetDeviceQueue(m_Device, indices.GraphicsFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, indices.PresentFamily.value(), 0, &m_PresentQueue);
    }
    void VulkanContext::CreateSurface()
    {
        VK_CHECK_RESULT(glfwCreateWindowSurface(m_Instance, m_WindowHandle, nullptr, &m_Surface));
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
    void VulkanContext::CheckInstanceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        for (int i = m_InstanceExtensions.size() - 1; i >= 0; i--)
        {
            bool found = false;
            for (const auto& extension : availableExtensions)
            {
                if (strcmp(m_InstanceExtensions[i], extension.extensionName) == 0)
                {
					found = true;
					break;
				}
			}
            if (found)
            {
                continue;
            }
            else
            {
				m_InstanceExtensions.erase(m_InstanceExtensions.begin() + i);
            }
        }
#ifdef GE_DEBUG
        for (const auto& extension : m_InstanceExtensions)
        {
			GE_CORE_INFO("Instance Extension Support: {0}", extension);
		}
#endif
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

        VK_CHECK_RESULT(vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_SwapChain));

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
            Utils::CreateImageViews(m_Device, m_SwapChainImages[i], m_SwapChainImageFormat, VK_IMAGE_VIEW_TYPE_2D, 1, VK_IMAGE_ASPECT_COLOR_BIT, 1, m_SwapChainImageViews[i]);
        }
    }
    void VulkanContext::CreateCommandBuffers()
    {
        m_CommandBuffer         = VulkanCommandBuffer(FindQueueFamilies(m_PhysicalDevice), m_CommandBufferSizePerFrame * Renderer::GetFramesInFlight());
        for (int i = 0; i < Renderer::GetFramesInFlight(); i++)
        {
            m_DrawUsedCommandBufferIndexs.push_back(0);
        }
    }

    void VulkanContext::BeginDrawCommandBuffer()
    {
        m_DrawUsedCommandBufferIndexs.at(Renderer::GetCurrentFrame()) = (m_DrawUsedCommandBufferIndexs.at(Renderer::GetCurrentFrame()) + 1) % m_CommandBufferSizePerFrame;
    }
    VkCommandBuffer VulkanContext::EndDrawCommandBuffer()
    {
        VkCommandBuffer     buffer = m_CommandBuffer.GetCommandBuffer(m_DrawUsedCommandBufferIndexs.at(Renderer::GetCurrentFrame()) + m_CommandBufferSizePerFrame * Renderer::GetCurrentFrame());
        return buffer;
    }
    VkCommandBuffer VulkanContext::GetCurrentDrawCommandBuffer()
    {
        VkCommandBuffer cmdBuffer = GetCurrentSecondaryDrawCommandBuffer();
        cmdBuffer = cmdBuffer != nullptr ? cmdBuffer : m_CommandBuffer.GetCommandBuffer(m_DrawUsedCommandBufferIndexs.at(Renderer::GetCurrentFrame()) + m_CommandBufferSizePerFrame * Renderer::GetCurrentFrame());
        return cmdBuffer;
    }
    void VulkanContext::BeginSecondaryDrawCommandBuffer()
    {
        // 根据当前thread id 分配secondary command buffer
        GE_CORE_ASSERT(m_DrawUsedSecondaryCommandBuffers.size() < m_CommandBuffer.GetSecondaryCommandBuffersSize(), "There is no avilable secondary command buffer");
        bool isUsing = true;
        while (isUsing)
        {
            m_DrawUsedSecondaryCommandBufferIndex = (m_DrawUsedSecondaryCommandBufferIndex + 1) % m_CommandBuffer.GetSecondaryCommandBuffersSize();
            for (int i = 0; i < m_DrawUsedSecondaryCommandBuffers.size(); i++)
            {
                if (m_DrawUsedSecondaryCommandBuffers.at(i).second == m_DrawUsedSecondaryCommandBufferIndex)
                {
                    isUsing = false;
                    break;
                }
            }
            isUsing = !isUsing;
            if (isUsing == false)
            {
                m_DrawUsedSecondaryCommandBuffers.emplace_back(std::this_thread::get_id(), m_DrawUsedSecondaryCommandBufferIndex);
            }
        }
    }
    VkCommandBuffer VulkanContext::EndSecondaryDrawCommandBuffer()
    {
		VkCommandBuffer cmdBuffer = nullptr;
		for (int i = 0; i < m_DrawUsedSecondaryCommandBuffers.size(); i++)
		{
			if (m_DrawUsedSecondaryCommandBuffers.at(i).first == std::this_thread::get_id())
			{
				cmdBuffer = m_CommandBuffer.GetSecondaryCommandBuffer(m_DrawUsedSecondaryCommandBuffers.at(i).second);
                m_DrawUsedSecondaryCommandBuffers.erase(m_DrawUsedSecondaryCommandBuffers.begin() + i);
				break;
			}
		}
		return cmdBuffer;
    }
    VkCommandBuffer VulkanContext::GetCurrentSecondaryDrawCommandBuffer()
    {
        VkCommandBuffer cmdBuffer = nullptr;
        for (int i = 0; i < m_DrawUsedSecondaryCommandBuffers.size(); i++)
        {
            if (m_DrawUsedSecondaryCommandBuffers.at(i).first == std::this_thread::get_id())
            {
                cmdBuffer = m_CommandBuffer.GetSecondaryCommandBuffer(m_DrawUsedSecondaryCommandBuffers.at(i).second);
                break;
            }
        }
        return cmdBuffer;
    }
	void VulkanContext::CreateDescriptor()
	{
        m_Descriptor            = VulkanDescriptor(1000, 100);
	}

    void VulkanContext::CreateSyncObjects()
    {
        m_Semaphores.resize(m_SyncObjectSizePerFrame * Renderer::GetFramesInFlight());
        m_Fences.resize(m_SyncObjectSizePerFrame * Renderer::GetFramesInFlight());

        for (int i = 0; i < m_Semaphores.size(); i++)
        {
            VkSemaphoreCreateInfo   semaphoreInfo{};
            semaphoreInfo.sType     = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            VK_CHECK_RESULT(vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_Semaphores[i]));
        }
        for (int i = 0; i < m_Fences.size(); i++)
        {
            VkFenceCreateInfo       fenceInfo{};
            fenceInfo.sType         = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags         = VK_FENCE_CREATE_SIGNALED_BIT;
            VK_CHECK_RESULT(vkCreateFence(m_Device, &fenceInfo, nullptr, &m_Fences[i]));
        }

        for (int i = 0; i < Renderer::GetFramesInFlight(); i++)
        {
            m_SemaphoreIndexs.push_back(0);
        }
    }

    void VulkanContext::CleanUpSwapChain()
    {
        vkDeviceWaitIdle(m_Device);
        m_SwapChainFrameBuffers.clear();
		for (auto imageView : m_SwapChainImageViews)
		{
			vkDestroyImageView(m_Device, imageView, nullptr);
		}
		vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);
    }

    void VulkanContext::LoadFunctionEXT(std::vector<const char*> ext)
    {
		if (std::find(ext.begin(), ext.end(), VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME) != ext.end())
		{
			// load vkCmdSetRasterizationSamplesEXT
			m_Function.vkCmdSetRasterizationSamplesEXT = (PFN_vkCmdSetRasterizationSamplesEXT)vkGetDeviceProcAddr(m_Device, "vkCmdSetRasterizationSamplesEXT");
            if(m_Function.vkCmdSetRasterizationSamplesEXT)
                GE_CORE_INFO("Load Functoin (vkCmdSetRasterizationSamplesEXT)");

            m_Function.vkCmdSetColorWriteMaskEXT = (PFN_vkCmdSetColorWriteMaskEXT)vkGetDeviceProcAddr(m_Device, "vkCmdSetColorWriteMaskEXT");
            if(m_Function.vkCmdSetColorWriteMaskEXT)
                GE_CORE_INFO("Load Functoin (vkCmdSetColorWriteMaskEXT)");
		}
    }

    void VulkanContext::RecreateSwapChain(unsigned int width, unsigned int height)
    {
        CleanUpSwapChain();

        CreateSwapChain(width, height);
        CreateImageViews();
        CreateFrameBuffer();
    }

	VkCommandBuffer VulkanContext::BeginSingleTimeCommands()
	{
        return m_CommandBuffer.BeginSingleTimeCommands();
	}

	void VulkanContext::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
        m_CommandBuffer.EndSingleTimeCommands(commandBuffer, m_GraphicsQueue);
	}

	void VulkanContext::CreateFrameBuffer()
	{
        m_SwapChainFrameBuffers.resize(m_SwapChainImageViews.size());
		for (size_t i = 0; i < m_SwapChainImageViews.size(); i++) {
            std::vector<VkImageView> attachmentsVec = { m_SwapChainImageViews[i] };
            std::vector<VkImage> imagesVec = { m_SwapChainImages[i] };

            FrameBufferSpecificationForVulkan   spec;
            spec.ColorImages                    = imagesVec;
            spec.ColorAttachments               = attachmentsVec;
            spec.ColorAttachmentsFormat         = { m_SwapChainImageFormat };
            spec.Width                          = m_SwapChainExtent.width;
            spec.Height                         = m_SwapChainExtent.height;
            spec.ColorAttachmentsFinalLayout    = { VK_IMAGE_LAYOUT_PRESENT_SRC_KHR };
            spec.EnableDepthStencilAttachment   = true;
            spec.Samples                        = 1;
            spec.AttachmentsBeginAction         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            spec.AttachmentsEndAction           = VK_ATTACHMENT_STORE_OP_STORE;

			Ref<VulkanFrameBuffer> frameBuffer  = CreateRef<VulkanFrameBuffer>(spec);
			m_SwapChainFrameBuffers[i]          = frameBuffer;
		}
	}
}