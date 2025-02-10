#pragma once
#include "GEngine/Renderer/Sampler.h"

namespace GEngine
{
	class GENGINE_API OpenGLSampler : public Sampler
	{
	public:
		OpenGLSampler(const SamplerSpecification& spec);
		virtual ~OpenGLSampler() override;

		virtual void Bind(uint32_t slot) override;

	private:
		uint32_t m_Sampler;
	};
}


