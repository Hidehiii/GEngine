#pragma once
#include "GEngine/Graphics/Texture.h"
#include "Platform/Vulkan/VulkanCubeMap.h"
#include "Platform/Vulkan/VulkanSampler.h"

namespace GEngine
{
    class GENGINE_API VulkanCubeMapCombineSampler :  public CubeMapCombineSampler
    {
    public:
		VulkanCubeMapCombineSampler(const Ref<CubeMap>& cubemap, const Ref<Sampler>& sampler);
		virtual void Bind(CommandBuffer* cmdBuffer, const uint32_t slot) override;
		virtual void			SetCubeMap(const Ref<CubeMap>& cubemap) override;
		virtual Ref<CubeMap>	GetCubeMap() override { return m_CubeMap; }
		virtual void			SetSampler(const Ref<Sampler>& sampler) override;
		virtual Ref<Sampler>	GetSampler() override { return m_Sampler; }
	private:
		Ref<VulkanCubeMap>	m_CubeMap;
		Ref<VulkanSampler>	m_Sampler;
    };
}


