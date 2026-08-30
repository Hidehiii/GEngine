#pragma once

#include "GEngine/Graphics/Texture.h"

namespace GEngine
{
	class GENGINE_API D3D12Texture2DCombineSampler : public Texture2DCombineSampler
	{
	public:
		D3D12Texture2DCombineSampler(const Ref<Texture2D>& texture, const Ref<Sampler>& sampler) : m_Texture(texture), m_Sampler(sampler) {}
		virtual void Bind(CommandBuffer* cmdBuffer, const uint32_t slot = 0) override {}
		virtual void SetTexture(const Ref<Texture2D>& texture) override { m_Texture = texture; }
		virtual Ref<Texture2D> GetTexture() override { return m_Texture; }
		virtual void SetSampler(const Ref<Sampler>& sampler) override { m_Sampler = sampler; }
		virtual Ref<Sampler> GetSampler() override { return m_Sampler; }
		virtual bool operator==(const Texture2DCombineSampler& other) const override
		{
			auto d3dOther = dynamic_cast<const D3D12Texture2DCombineSampler*>(&other);
			return d3dOther && m_Texture == d3dOther->m_Texture && m_Sampler == d3dOther->m_Sampler;
		}
	private:
		Ref<Texture2D> m_Texture;
		Ref<Sampler> m_Sampler;
	};

	class GENGINE_API D3D12CubeMapCombineSampler : public CubeMapCombineSampler
	{
	public:
		D3D12CubeMapCombineSampler(const Ref<CubeMap>& cubeMap, const Ref<Sampler>& sampler) : m_CubeMap(cubeMap), m_Sampler(sampler) {}
		virtual void Bind(CommandBuffer* cmdBuffer, const uint32_t slot = 0) override {}
		virtual void SetCubeMap(const Ref<CubeMap>& cubeMap) override { m_CubeMap = cubeMap; }
		virtual Ref<CubeMap> GetCubeMap() override { return m_CubeMap; }
		virtual void SetSampler(const Ref<Sampler>& sampler) override { m_Sampler = sampler; }
		virtual Ref<Sampler> GetSampler() override { return m_Sampler; }
		virtual bool operator==(const CubeMapCombineSampler& other) const override
		{
			auto d3dOther = dynamic_cast<const D3D12CubeMapCombineSampler*>(&other);
			return d3dOther && m_CubeMap == d3dOther->m_CubeMap && m_Sampler == d3dOther->m_Sampler;
		}
	private:
		Ref<CubeMap> m_CubeMap;
		Ref<Sampler> m_Sampler;
	};
}
