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

		VkDescriptorImageInfo*			GetDescriptorImageInfo() { return &m_SamplerInfo; }
		VkSampler						GetVulkanSampler() { return m_Sampler; }

		virtual bool operator==(const Sampler& other) const override
		{
			auto otherVulkan = static_cast<const VulkanSampler&>(other);
			return m_Specification == otherVulkan.m_Specification &&
				m_Sampler == otherVulkan.m_Sampler;
		}
	private:
		VkSampler						m_Sampler = nullptr;
		VkDescriptorImageInfo			m_SamplerInfo = {};
	};
}


