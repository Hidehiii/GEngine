#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/GraphicsContext.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanDescriptor.h"
#include "Platform/Vulkan/VulkanFrameBuffer.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Renderer/Renderer.h"
#include <Platform/Vulkan/VMA/vk_mem_alloc.h>
#include <GLFW/glfw3.h>


namespace GEngine
{
	struct VulkanFunctionEXT
	{
		PFN_vkCmdSetRasterizationSamplesEXT vkCmdSetRasterizationSamplesEXT = VK_NULL_HANDLE;
		PFN_vkCmdSetColorWriteMaskEXT		vkCmdSetColorWriteMaskEXT		= VK_NULL_HANDLE;
		PFN_vkCmdSetColorBlendEquationEXT	vkCmdSetColorBlendEquationEXT	= VK_NULL_HANDLE;
		PFN_vkCmdSetColorBlendEnableEXT		vkCmdSetColorBlendEnableEXT		= VK_NULL_HANDLE;
		PFN_vkCmdSetPolygonModeEXT			vkCmdSetPolygonModeEXT			= VK_NULL_HANDLE;
	};
	

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
		virtual void				SetVSync(bool enable) override;
		virtual void				SetRequiredExtensions(std::vector<const char*> extensions) override { m_Extensions = extensions; }

		void						RecreateSwapChain(unsigned int width, unsigned int height);
		static VulkanContext*		Get() { return s_ContextInstance; }
		VmaAllocator				GetVmaAllocator() { return m_VmaAllocator; }
		VkDevice					GetDevice() { return m_Device; }
		VkExtent2D					GetSwapChainExtent() { return m_SwapChainExtent; }
		std::vector<VkImage>		GetSwapChainImage() { return m_SwapChainImages; }
		VkPhysicalDevice			GetPhysicalDevice() { return m_PhysicalDevice; }
		void						BeginGraphicsCommandBuffer();
		VkCommandBuffer				EndGraphicsCommandBuffer();
		void						BeginComputeCommandBuffer();
		VkCommandBuffer				EndComputeCommandBuffer();
		VkCommandBuffer				GetCurrentCommandBuffer();
		void						BeginSecondaryCommandBuffer();
		VkCommandBuffer				EndSecondaryCommandBuffer();
		VkCommandBuffer				GetCurrentSecondaryCommandBuffer();
		void						SetClearColor(Vector4 color) { m_ClearColor = color; }
		Vector4						GetClearColor() { return m_ClearColor; }
		VkInstance					GetInstance() { return m_Instance; }
		VkDescriptorPool			GetDescriptorPool() { return m_Descriptor.GetDescriptorPool(); }
		VkCommandBuffer				BeginSingleTimeGraphicsCommand();
		void						EndSingleTimeGraphicsCommand(VkCommandBuffer commandBuffer);
		VkCommandBuffer				BeginSingleTimeComputeCommand();
		void						EndSingleTimeComputeCommand(VkCommandBuffer commandBuffer);
		QueueFamilyIndices			GetQueueFamily() { return m_QueueFamily; }
		VkQueue						GetGraphicsQueue() { return m_GraphicsQueue; }
		VkQueue						GetPresentQueue() { return m_PresentQueue; }
		VkQueue						GetComputeQueue() { return m_ComputeQueue; }
		VkSwapchainKHR				GetSwapChain() { return m_SwapChain; }
		Ref<VulkanFrameBuffer>		GetFrameBuffer(int index) { return m_SwapChainFrameBuffers[index % m_SwapChainFrameBuffers.size()]; }
		VkFence&					GetCurrentFence();
		VkSemaphore&				GetCurrentSemaphore();
		void                        MoveToNextSemaphore();
		VkSemaphore&				GetSemaphore(int index) { return m_Semaphores.at(index % m_Semaphores.size()); }
		VulkanFunctionEXT&			GetVulkanFunctionEXT() { return m_Function; }
		VkCommandPool				GetGraphicsCommandPool() { return m_CommandBufferPool.GetGraphicsCommandPool(); }
	protected:
		Ref<VulkanCommandBuffer>	GetCommandBuffer(CommandBufferType type);
	private:
		void						CreateInstance();
		bool						CheckValidationLayerSupport();
		void						SetupDebugMessenger();
		void						PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void						SetPhysicalDevice();
		bool						IsDeviceSuitable(VkPhysicalDevice device);
		void						CreateLogicalDevice();
		void						CreateVmaAllocator();
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
		void						CreateRenderPass();
		void						CreateFrameBuffer();
		void						CreateCommandBuffers();
		void						CreateDescriptor();
		void						CreateSyncObjects();
		void						CleanUpSwapChain();
		void						LoadFunctionEXT(std::vector<const char*> ext);
	private:
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
		VmaAllocator						m_VmaAllocator;
		VkQueue								m_GraphicsQueue;
		VkQueue								m_PresentQueue;
		VkQueue								m_ComputeQueue;
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
			VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME,
			VK_KHR_16BIT_STORAGE_EXTENSION_NAME,
			VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME,
			VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
			VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
			VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME,
			VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
			VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME,
			VK_KHR_RAY_QUERY_EXTENSION_NAME,
		};
		bool								m_VSync = false;
		VkSwapchainKHR						m_SwapChain;
		std::vector<VkImage>				m_SwapChainImages;
		VkFormat							m_SwapChainImageFormat;
		VkExtent2D							m_SwapChainExtent;
		std::vector<VkImageView>			m_SwapChainImageViews;
		Ref<VulkanRenderPass>				m_SwapChainRenderPass;
		std::vector<Ref<VulkanFrameBuffer>>	m_SwapChainFrameBuffers;
		VulkanCommandBufferPool				m_CommandBufferPool;
		CommandBufferType					m_CurrentCmdBufferType = CommandBufferType::Graphics;
		std::vector<int>					m_UsedGraphicsCommandBufferIndexs;
		std::vector<int>					m_UsedComputeCommandBufferIndexs;
		Vector4								m_ClearColor = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		VulkanDescriptor					m_Descriptor;
		QueueFamilyIndices					m_QueueFamily;
		std::vector<VkSemaphore>			m_Semaphores;
		std::vector<int>                    m_SemaphoreIndexs;
		std::vector<VkFence>				m_Fences;
		VulkanFunctionEXT					m_Function;
		std::vector<std::pair<std::thread::id, int>> m_UsedSecondaryCommandBuffers;
		int									m_UsedSecondaryCommandBufferIndex = 0;

		std::vector<Ref<VulkanCommandBuffer>>	m_GraphicsCommandBuffers;
		uint32_t							m_GraphicsCommandBufferIndex = 0;
		std::vector<Ref<VulkanCommandBuffer>>	m_ComputeCommandBuffers;
		uint32_t							m_ComputeCommandBufferIndex = 0;

		friend class VulkanRendererAPI;
	};
	
}


