#pragma once

#include "GEngine/Core/Core.h"

namespace GEngine
{
	enum class SamplerFilter
	{
		Liner,
		Nearest,
	};

	enum class SamplerAddressMode
	{
		Repeat,
		MirroredRepeat,
		ClampToEdge,
		ClampToBorder,
		MirrorClampToEdge,
	};

	struct SamplerSpecification
	{
		SamplerFilter MagFilter = SamplerFilter::Liner;
		SamplerFilter MinFilter = SamplerFilter::Liner;
		SamplerFilter MipmapFilter = SamplerFilter::Liner;

		SamplerAddressMode WrapU = SamplerAddressMode::Repeat;
		SamplerAddressMode WrapV = SamplerAddressMode::Repeat;
		SamplerAddressMode WrapW = SamplerAddressMode::Repeat;

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


