#include "GEpch.h"
#include "VulkanDescriptor.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace GEngine
{
	VulkanDescriptor::VulkanDescriptor(uint32_t descriptorCount, uint32_t maxSets)
	{
		CreateDescriptorPool(descriptorCount, maxSets);
	}
	void VulkanDescriptor::CreateDescriptorPool(uint32_t descriptorCount, uint32_t maxSets)
	{
		std::vector<VkDescriptorPoolSize> poolSizes =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER,					descriptorCount },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	descriptorCount },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,				descriptorCount },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,				descriptorCount },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,		descriptorCount },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,		descriptorCount },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,			descriptorCount },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,			descriptorCount },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,	descriptorCount },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,	descriptorCount },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,			descriptorCount }
		};


		VkDescriptorPoolCreateInfo	poolInfo{};
		poolInfo.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount		= static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes			= poolSizes.data();
		poolInfo.maxSets			= maxSets;
		poolInfo.flags				= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		VK_CHECK_RESULT(vkCreateDescriptorPool(VulkanContext::Get()->GetDevice(), &poolInfo, nullptr, &m_DescriptorPool));
	}

	void VulkanDescriptor::Release()
	{
		vkDestroyDescriptorPool(VulkanContext::Get()->GetDevice(), m_DescriptorPool, nullptr);
	}
}