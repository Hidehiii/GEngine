#pragma once
#include "GEngine/Core/Core.h"

#include <vulkan/vulkan.h>

namespace GEngine
{
	struct DescriptorPoolSpecification
	{

	};

	class GENGINE_API VulkanDescriptor
	{
	public:
		VulkanDescriptor() = default;
		VulkanDescriptor(uint32_t descriptorCount, uint32_t maxSets);

		VkDescriptorPool GetDescriptorPool() { return m_DescriptorPool; }
		void Release();
	private:
		void CreateDescriptorPool(uint32_t descriptorCount, uint32_t maxSets);

	private:
		VkDescriptorPool		m_DescriptorPool;
	};
}


