#pragma once
#include "GEngine/Graphics/Sampler.h"

namespace GEngine
{
	class GENGINE_API OpenGLSampler : public Sampler
	{
	public:
		OpenGLSampler(const SamplerSpecification& spec);
		virtual ~OpenGLSampler() override;

		virtual void Bind(uint32_t slot) override;

		virtual bool operator==(const Sampler& other) const override
		{
			auto s = static_cast<const OpenGLSampler&>(other);
			return m_Specification == other.GetSpecification() &&
				m_Sampler == s.m_Sampler;
		}
	private:
		uint32_t m_Sampler;
	};
}


