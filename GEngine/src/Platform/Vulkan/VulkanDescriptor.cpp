#include "GEpch.h"
#include "VulkanDescriptor.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace GEngine
{
	VulkanDescriptor::VulkanDescriptor(uint32_t descriptorCount, uint32_t poolSizeCount, uint32_t maxSets)
	{
		CreateDescriptorPool(descriptorCount, poolSizeCount, maxSets);
	}
	void VulkanDescriptor::CreateDescriptorPool(uint32_t descriptorCount, uint32_t poolSizeCount, uint32_t maxSets)
	{
		std::vector<VkDescriptorPoolSize>	poolSizes;
		for (int i = 0; i < poolSizeCount; i++)
		{
			VkDescriptorPoolSize		poolSize{};
			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSize.descriptorCount = descriptorCount;

			poolSizes.push_back(poolSize);
		}

		VkDescriptorPoolCreateInfo	poolInfo{};
		poolInfo.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount		= static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes			= poolSizes.data();
		poolInfo.maxSets			= maxSets;

		VK_CHECK_RESULT(vkCreateDescriptorPool(VulkanContext::GetDevice(), &poolInfo, nullptr, &m_DescriptorPool));
	}

	void VulkanDescriptor::Release()
	{
		vkDestroyDescriptorPool(VulkanContext::GetDevice(), m_DescriptorPool, nullptr);
	}
}