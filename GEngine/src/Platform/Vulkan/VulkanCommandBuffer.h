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
		VulkanCommandBuffer(QueueFamilyIndices queueFamilyIndices, int count = 10);
		~VulkanCommandBuffer();

		VkCommandBuffer GetCommandBuffer(int index = 0) { GE_CORE_ASSERT(index < m_CommandBuffers.size(), "Out of range!"); return m_CommandBuffers.at(index); }
		uint32_t GetCommandBuffersSize() { return m_CommandBuffers.size(); }
	private:
		void CreateCommandPool(QueueFamilyIndices queueFamilyIndices);
	private:
		VkCommandPool					m_CommandPool;
		std::vector<VkCommandBuffer>	m_CommandBuffers;
	};
}


