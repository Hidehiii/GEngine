#pragma once

#include "GEngine/Core/Core.h"

namespace GEngine
{
	

	class GENGINE_API Sampler
	{
	public:
		virtual ~Sampler() = default;

		virtual void Bind(uint32_t slot) = 0;

		virtual SamplerSpecification GetSpecification() { return m_Specification; }

		static Ref<Sampler> GetSampler(const SamplerSpecification& spec);

		static Ref<Sampler> Create(const SamplerSpecification& spec);
	protected:
		SamplerSpecification	m_Specification;
	protected:
		static std::vector<Ref<Sampler>>	s_Samplers;
	};
}


