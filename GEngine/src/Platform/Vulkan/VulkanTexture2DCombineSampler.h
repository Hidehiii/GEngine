#pragma once
#include "GEngine/Graphics/Texture.h"
#include "Platform/Vulkan/VulkanTexture2D.h"
#include "Platform/Vulkan/VulkanSampler.h"

namespace GEngine
{
	class GENGINE_API VulkanTexture2DCombineSampler : public Texture2DCombineSampler
	{
	public:
		VulkanTexture2DCombineSampler(const Ref<Texture2D>& texture, const Ref<Sampler>& sampler);

		virtual void Bind(CommandBuffer* cmdBuffer, const uint32_t slot) override;

		virtual void			SetTexture(const Ref<Texture2D>& texture) override;
		virtual Ref<Texture2D>	GetTexture() override { return m_Texture; }
		virtual void			SetSampler(const Ref<Sampler>& sampler) override;
		virtual Ref<Sampler>	GetSampler() override { return m_Sampler; }

		VkDescriptorImageInfo*	GetDescriptorImageInfo() { return &m_ImageInfo; }

	private:
		Ref<VulkanTexture2D>	m_Texture;
		Ref<VulkanSampler>		m_Sampler;
		
		VkDescriptorImageInfo	m_ImageInfo{};
	};
}


