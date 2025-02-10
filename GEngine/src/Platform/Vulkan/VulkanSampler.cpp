#include "GEpch.h"
#include "VulkanSampler.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace GEngine
{
    VulkanSampler::VulkanSampler(const SamplerSpecification& spec)
    {
        m_Specification = spec;

		VkSamplerCreateInfo             samplerInfo{};
		samplerInfo.sType				= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter			= Utils::SamplerFilterToVkFilter(m_Specification.MagFilter);
		samplerInfo.minFilter			= Utils::SamplerFilterToVkFilter(m_Specification.MinFilter);
		samplerInfo.addressModeU		= Utils::SamplerAddressModeToVkSamplerAddressMode(m_Specification.WrapU);
		samplerInfo.addressModeV		= Utils::SamplerAddressModeToVkSamplerAddressMode(m_Specification.WrapV);
		samplerInfo.addressModeW		= Utils::SamplerAddressModeToVkSamplerAddressMode(m_Specification.WrapW);
		samplerInfo.anisotropyEnable	= VK_TRUE;

		VkPhysicalDeviceProperties		properties{};
		vkGetPhysicalDeviceProperties(VulkanContext::Get()->GetPhysicalDevice(), &properties);

		samplerInfo.maxAnisotropy			= properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor				= VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable			= VK_FALSE;
		samplerInfo.compareOp				= VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode				= Utils::SamplerFilterToVkSamplerMipmapMode(m_Specification.MipmapFilter);
		samplerInfo.mipLodBias				= 0.0f;
		samplerInfo.minLod					= 0.0f;
		samplerInfo.maxLod					= VK_LOD_CLAMP_NONE;

		VK_CHECK_RESULT(vkCreateSampler(VulkanContext::Get()->GetDevice(), &samplerInfo, nullptr, &m_Sampler));

		m_SamplerInfo.sampler = m_Sampler;

		m_DescriptorSetLayoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_SAMPLER;
		m_DescriptorSetLayoutBinding.descriptorCount	= 1;
		m_DescriptorSetLayoutBinding.stageFlags			= VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT;
		m_DescriptorSetLayoutBinding.pImmutableSamplers = nullptr;
    }
    VulkanSampler::~VulkanSampler()
    {
        if (VulkanContext::Get()->GetDevice())
        {
            vkDestroySampler(VulkanContext::Get()->GetDevice(), m_Sampler, nullptr);
        }
    }
    void VulkanSampler::Bind(uint32_t slot)
    {
		m_DescriptorSetLayoutBinding.binding = slot;
    }
}