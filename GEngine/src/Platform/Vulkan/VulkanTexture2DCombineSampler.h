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

	private:
		Ref<VulkanTexture2D> m_Texture;
		Ref<VulkanSampler>   m_Sampler;
	};
}


