#pragma once

#include "GEngine/Graphics/Sampler.h"
#include <d3d12.h>

namespace GEngine
{
	class GENGINE_API D3D12Sampler : public Sampler
	{
	public:
		explicit D3D12Sampler(const SamplerSpecification& spec);
		virtual void Bind(uint32_t slot) override {}
		const D3D12_SAMPLER_DESC& GetDescriptor() const { return m_Descriptor; }
		virtual bool operator==(const Sampler& other) const override;
	private:
		D3D12_SAMPLER_DESC m_Descriptor{};
	};
}
