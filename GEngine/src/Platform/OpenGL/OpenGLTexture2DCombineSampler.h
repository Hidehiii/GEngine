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

	private:
		Ref<OpenGLTexture2D> m_Texture;
		Ref<OpenGLSampler>   m_Sampler;
	};
}


