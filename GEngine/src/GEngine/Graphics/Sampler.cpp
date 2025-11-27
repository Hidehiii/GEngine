#include "GEpch.h"
#include "Sampler.h"
#include "Graphics.h"
#include "Platform/OpenGL/OpenGLSampler.h"
#include "Platform/Vulkan/VulkanSampler.h"

namespace GEngine
{
	std::vector<Ref<Sampler>> Sampler::s_Samplers;

	Ref<Sampler> Sampler::GetSampler(const SamplerSpecification& spec)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_NONE: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GRAPHICS_API_OPENGL: {
			Ref<Sampler> sampler = GetSampler(spec);
			if (sampler == nullptr)
			{
				sampler = CreateRef<OpenGLSampler>(spec);
				s_Samplers.push_back(sampler);
			}
			return sampler;
		}
		case GRAPHICS_API_VULKAN: {
			Ref<Sampler> sampler = GetSampler(spec);
			if (sampler == nullptr)
			{
				sampler = CreateRef<VulkanSampler>(spec);
				s_Samplers.push_back(sampler);
			}
			return sampler;
		}
		}

		GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
		return nullptr;
	}
	Ref<Sampler> Sampler::Create(const SamplerSpecification& spec)
	{
		for (auto sampler : s_Samplers)
		{
			if (sampler->GetSpecification() == spec)
			{
				return sampler;
			}
		}
		return nullptr;
	}
}