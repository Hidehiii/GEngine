#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsCommon.h"
namespace GEngine
{
	

	class GENGINE_API Sampler
	{
	public:
		virtual ~Sampler() = default;

		virtual void Bind(uint32_t slot) = 0;

		virtual SamplerSpecification GetSpecification() const { return m_Specification; }

		static Ref<Sampler> GetSampler(const SamplerSpecification& spec);
		static Ref<Sampler> GetDefaultSampler();

		static Ref<Sampler> Create(const SamplerSpecification& spec);

		virtual bool operator==(const Sampler& other) const = 0;
	protected:
		SamplerSpecification				m_Specification;
	protected:
		static std::unordered_map<SamplerSpecification, Ref<Sampler>>	s_Samplers;
	};
}


