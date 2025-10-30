#pragma once

#include "GEngine/Core/Core.h"

namespace GEngine
{
	enum SamplerFilter
	{
		SAMPLER_FILTER_LINEAR,
		SAMPLER_FILTER_NEAREST,
	};

	enum SamplerAddressMode
	{
		SAMPLER_ADDRESS_REPEAT,
		SAMPLER_ADDRESS_MIRRORED_REPEAT,
		SAMPLER_ADDRESS_CLAMP_TO_EDGE,
		SAMPLER_ADDRESS_CLAMP_TO_BORDER,
		SAMPLER_ADDRESS_MIRROR_CLAMP_TO_EDGE,
	};

	struct SamplerSpecification
	{
		SamplerFilter MagFilter		= SAMPLER_FILTER_LINEAR;
		SamplerFilter MinFilter		= SAMPLER_FILTER_LINEAR;
		SamplerFilter MipmapFilter	= SAMPLER_FILTER_LINEAR;

		SamplerAddressMode WrapU = SAMPLER_ADDRESS_REPEAT;
		SamplerAddressMode WrapV = SAMPLER_ADDRESS_REPEAT;
		SamplerAddressMode WrapW = SAMPLER_ADDRESS_REPEAT;

		bool operator==(const SamplerSpecification& other) const
		{
			return MagFilter == other.MagFilter &&
				MinFilter == other.MinFilter &&
				MipmapFilter == other.MipmapFilter &&
				WrapU == other.WrapU &&
				WrapV == other.WrapV &&
				WrapW == other.WrapW;
		}
	};

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


