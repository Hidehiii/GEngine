#pragma once
#include "GEngine/Core/Core.h"
#include <vulkan/vulkan.h>
#include <optional>
namespace GEngine
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> GraphicsFamily;
		std::optional<uint32_t> PresentFamily;
		std::optional<uint32_t>	ComputeFamily;

		bool IsComplete()
		{
			return GraphicsFamily.has_value() && PresentFamily.has_value() && ComputeFamily.has_value();
		}

	};

	class GENGINE_API VulkanCommandBuffer
	{
	public:
		VulkanCommandBuffer() = default;
		VulkanCommandBuffer(QueueFamilyIndices queueFamilyIndices, int count = 10);
		~VulkanCommandBuffer();
		void Release();

		VkCommandBuffer GetGraphicsCommandBuffer(int index = 0) { return m_GraphicsCommandBuffers.at(index); }
		VkCommandBuffer GetComputeCommandBuffer(int index = 0) { return m_ComputeCommandBuffers.at(index); }
		VkCommandBuffer GetSecondaryCommandBuffer(int index = 0) { return m_SecondaryCommandBuffers.at(index); }
		uint32_t		GetCommandBuffersSize() { return m_GraphicsCommandBuffers.size(); }
		uint32_t		GetSecondaryCommandBuffersSize() { return m_SecondaryCommandBuffers.size(); }

		VkCommandBuffer BeginSingleTimeGraphicsCommands();
		void			EndSingleTimeGraphicsCommands(VkCommandBuffer commandBuffer, VkQueue queue);
	private:
		void			CreateCommandPool(QueueFamilyIndices queueFamilyIndices);
	private:
		VkCommandPool					m_GraphicsCommandPool = VK_NULL_HANDLE;
		VkCommandPool					m_ComputeCommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer>	m_GraphicsCommandBuffers;
		std::vector<VkCommandBuffer>	m_ComputeCommandBuffers;
		std::vector<VkCommandBuffer>	m_SecondaryCommandBuffers;
	};
}


