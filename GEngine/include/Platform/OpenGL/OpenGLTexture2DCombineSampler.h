#pragma once
#include "GEngine/Graphics/Texture.h"
#include "Platform/OpenGL/OpenGLTexture2D.h"
#include "Platform/OpenGL/OpenGLSampler.h"

namespace GEngine
{
	class GENGINE_API OpenGLTexture2DCombineSampler : public Texture2DCombineSampler
	{
	public:
		OpenGLTexture2DCombineSampler(const Ref<Texture2D>& texture, const Ref<Sampler>& sampler);

		virtual void Bind(CommandBuffer* cmdBuffer, const uint32_t slot) override;

		virtual void			SetTexture(const Ref<Texture2D>& texture) override;
		virtual Ref<Texture2D>	GetTexture() override { return m_Texture; }
		virtual void			SetSampler(const Ref<Sampler>& sampler) override;
		virtual Ref<Sampler>	GetSampler() override { return m_Sampler; }

		virtual bool operator==(const Texture2DCombineSampler& other) const override
		{
			auto otherOpenGL = static_cast<const OpenGLTexture2DCombineSampler&>(other);
			return (*m_Texture == *otherOpenGL.m_Texture) && (*m_Sampler == *otherOpenGL.m_Sampler);
		}
	private:
		Ref<OpenGLTexture2D> m_Texture;
		Ref<OpenGLSampler>   m_Sampler;
	};
}


