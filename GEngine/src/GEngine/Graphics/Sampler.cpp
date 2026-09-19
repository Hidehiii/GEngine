#include "GEpch.h"
#include "GEngine/Graphics/Sampler.h"
#include "GEngine/Graphics/Graphics.h"

namespace GEngine
{
	std::unordered_map<SamplerSpecification, Ref<Sampler>> Sampler::s_Samplers;

	Ref<Sampler> Sampler::GetSampler(const SamplerSpecification& spec)
	{
		const auto found = s_Samplers.find(spec);
		if (found != s_Samplers.end()) return found->second;
		auto sampler = Graphics::GetRenderDevice().CreateSampler(spec);
		s_Samplers.emplace(spec, sampler);
		return sampler;
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
	void Sampler::ShutdownCache()
	{
		s_Samplers.clear();
	}
}
