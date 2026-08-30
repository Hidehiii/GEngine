#pragma once
#include "GEngine/Graphics/Texture.h"
#include "Platform/OpenGL/OpenGLCubeMap.h"
#include "Platform/OpenGL/OpenGLSampler.h"

namespace GEngine
{
	class GENGINE_API OpenGLCubeMapCombineSampler : public CubeMapCombineSampler
	{
	public:
		OpenGLCubeMapCombineSampler(const Ref<CubeMap>& cubemap, const Ref<Sampler>& sampler);
		virtual void Bind(CommandBuffer* cmdBuffer, const uint32_t slot) override;
		virtual void			SetCubeMap(const Ref<CubeMap>& cubemap) override;
		virtual Ref<CubeMap>	GetCubeMap() override { return m_CubeMap; }
		virtual void			SetSampler(const Ref<Sampler>& sampler) override;
		virtual Ref<Sampler>	GetSampler() override { return m_Sampler; }

		virtual bool operator==(const CubeMapCombineSampler& other) const override
		{
			auto otherOpenGL = static_cast<const OpenGLCubeMapCombineSampler&>(other);
			return (*m_CubeMap == *otherOpenGL.m_CubeMap) && (*m_Sampler == *otherOpenGL.m_Sampler);
		}

	private:
		Ref<OpenGLCubeMap>	m_CubeMap;
		Ref<OpenGLSampler>	m_Sampler;
	};
}


