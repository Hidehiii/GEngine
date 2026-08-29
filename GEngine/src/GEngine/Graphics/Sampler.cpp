#include "GEpch.h"
#include "Sampler.h"
#include "Graphics.h"
#include "Platform/OpenGL/OpenGLSampler.h"
#include "Platform/Vulkan/VulkanSampler.h"
#include "Platform/D3D12/D3D12Sampler.h"

namespace GEngine
{
	std::unordered_map<SamplerSpecification, Ref<Sampler>> Sampler::s_Samplers;

	Ref<Sampler> Sampler::GetSampler(const SamplerSpecification& spec)
	{
		if (s_Samplers.find(spec) != s_Samplers.end())
			return s_Samplers.at(spec);

		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_OPENGL: {
			Ref<Sampler> sampler = CreateRef<OpenGLSampler>(spec);
			s_Samplers.emplace(spec, sampler);
			return sampler;
		}
		case GRAPHICS_API_VULKAN: {
			Ref<Sampler> sampler = CreateRef<VulkanSampler>(spec);
			s_Samplers.emplace(spec, sampler);
			return sampler;
		}
		case GRAPHICS_API_DIRECT3DX12:
		{
			Ref<Sampler> sampler = CreateRef<D3D12Sampler>(spec);
			s_Samplers.emplace(spec, sampler);
			return sampler;
		}
		case GRAPHICS_API_NONE:
		default:
			GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
			return nullptr;
		}
	}
	Ref<Sampler> Sampler::GetDefaultSampler()
	{
		SamplerSpecification spec{};
		return GetSampler(spec);
	}
	Ref<Sampler> Sampler::Create(const SamplerSpecification& spec)
	{
		return GetSampler(spec);
	}
}
