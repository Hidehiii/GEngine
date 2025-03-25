#pragma once
#include "GEngine/Graphics/Sampler.h"
#include <vulkan/vulkan.h>
namespace GEngine
{
	class GENGINE_API VulkanSampler : public Sampler
	{
	public:
		VulkanSampler(const SamplerSpecification& spec);
		virtual ~VulkanSampler() override;

		virtual void Bind(uint32_t slot) override;

		VkDescriptorImageInfo			GetDescriptorImageInfo() { return m_SamplerInfo; }
		VkDescriptorSetLayoutBinding	GetDescriptorSetLayoutBinding() { return m_DescriptorSetLayoutBinding; }
	private:
		VkSampler						m_Sampler = nullptr;
		VkDescriptorImageInfo			m_SamplerInfo = {};
		VkDescriptorSetLayoutBinding	m_DescriptorSetLayoutBinding = {};
	};
}


