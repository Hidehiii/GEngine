#include "GEpch.h"
#include "Sampler.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLSampler.h"
#include "Platform/Vulkan/VulkanSampler.h"

namespace GEngine
{
	std::vector<Ref<Sampler>> Sampler::s_Samplers;

	Ref<Sampler> Sampler::GetSampler(const SamplerSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: {
			GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		}
		case RendererAPI::API::OpenGL: {
			Ref<Sampler> sampler = GetSampler(spec);
			if (sampler == nullptr)
			{
				sampler = CreateRef<OpenGLSampler>(spec);
				s_Samplers.push_back(sampler);
			}
			return sampler;
		}
		case RendererAPI::API::Vulkan: {
			Ref<Sampler> sampler = GetSampler(spec);
			if (sampler == nullptr)
			{
				sampler = CreateRef<VulkanSampler>(spec);
				s_Samplers.push_back(sampler);
			}
			return sampler;
		}
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
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