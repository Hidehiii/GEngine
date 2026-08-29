#include "GEpch.h"
#include "D3D12Sampler.h"

namespace GEngine
{
	static D3D12_FILTER_TYPE ToD3D12FilterType(SamplerFilter filter)
	{
		return filter == SAMPLER_FILTER_NEAREST ? D3D12_FILTER_TYPE_POINT : D3D12_FILTER_TYPE_LINEAR;
	}
	static D3D12_TEXTURE_ADDRESS_MODE ToD3D12AddressMode(SamplerAddressMode mode)
	{
		switch (mode)
		{
		case SAMPLER_ADDRESS_REPEAT: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		case SAMPLER_ADDRESS_MIRRORED_REPEAT: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		case SAMPLER_ADDRESS_CLAMP_TO_EDGE: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		case SAMPLER_ADDRESS_CLAMP_TO_BORDER: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		case SAMPLER_ADDRESS_MIRROR_CLAMP_TO_EDGE: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
		default: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		}
	}

	D3D12Sampler::D3D12Sampler(const SamplerSpecification& spec)
	{
		m_Specification = spec;
		m_Descriptor.Filter = D3D12_ENCODE_BASIC_FILTER(
			ToD3D12FilterType(spec.MinFilter),
			ToD3D12FilterType(spec.MagFilter),
			ToD3D12FilterType(spec.MipmapFilter),
			D3D12_FILTER_REDUCTION_TYPE_STANDARD);
		m_Descriptor.AddressU = ToD3D12AddressMode(spec.WrapU);
		m_Descriptor.AddressV = ToD3D12AddressMode(spec.WrapV);
		m_Descriptor.AddressW = ToD3D12AddressMode(spec.WrapW);
		m_Descriptor.MipLODBias = 0.0f;
		m_Descriptor.MaxAnisotropy = 1;
		m_Descriptor.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		m_Descriptor.BorderColor[0] = 0.0f;
		m_Descriptor.BorderColor[1] = 0.0f;
		m_Descriptor.BorderColor[2] = 0.0f;
		m_Descriptor.BorderColor[3] = 1.0f;
		m_Descriptor.MinLOD = 0.0f;
		m_Descriptor.MaxLOD = D3D12_FLOAT32_MAX;
	}

	bool D3D12Sampler::operator==(const Sampler& other) const
	{
		const auto& otherSampler = static_cast<const D3D12Sampler&>(other);
		return m_Specification == otherSampler.m_Specification;
	}
}
