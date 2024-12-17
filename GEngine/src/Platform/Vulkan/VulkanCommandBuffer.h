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

		bool IsComplete()
		{
			return GraphicsFamily.has_value() && PresentFamily.has_value();
		}

	};

	class GENGINE_API VulkanCommandBuffer
	{
	public:
		VulkanCommandBuffer() = default;
		VulkanCommandBuffer(QueueFamilyIndices queueFamilyIndices, int count = 10);
		~VulkanCommandBuffer();
		void Release();

		VkCommandBuffer GetCommandBuffer(int index = 0) { return m_CommandBuffers.at(index); }
		VkCommandBuffer GetSecondaryCommandBuffer(int index = 0) { return m_SecondaryCommandBuffers.at(index); }
		uint32_t		GetCommandBuffersSize() { return m_CommandBuffers.size(); }
		uint32_t		GetSecondaryCommandBuffersSize() { return m_SecondaryCommandBuffers.size(); }

		VkCommandBuffer BeginSingleTimeCommands();
		void			EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue);
	private:
		void			CreateCommandPool(QueueFamilyIndices queueFamilyIndices);
	private:
		VkCommandPool					m_CommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer>	m_CommandBuffers;
		std::vector<VkCommandBuffer>	m_SecondaryCommandBuffers;
	};
}


