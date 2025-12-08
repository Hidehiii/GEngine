#include "GEpch.h"
#include "OpenGLCubeMapCombineSampler.h"
#include <glad/glad.h>

namespace GEngine
{
	OpenGLCubeMapCombineSampler::OpenGLCubeMapCombineSampler(const Ref<CubeMap>& cubemap, const Ref<Sampler>& sampler)
	{
		m_CubeMap = std::dynamic_pointer_cast<OpenGLCubeMap>(cubemap);
		m_Sampler = std::dynamic_pointer_cast<OpenGLSampler>(sampler);
	}

	void OpenGLCubeMapCombineSampler::Bind(CommandBuffer* cmdBuffer, const uint32_t slot)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMap->GetOpenGLID());
		m_Sampler->Bind(slot);
	}

	void OpenGLCubeMapCombineSampler::SetCubeMap(const Ref<CubeMap>& cubemap)
	{
		m_CubeMap = std::dynamic_pointer_cast<OpenGLCubeMap>(cubemap);
	}

	void OpenGLCubeMapCombineSampler::SetSampler(const Ref<Sampler>& sampler)
	{
		m_Sampler = std::dynamic_pointer_cast<OpenGLSampler>(sampler);
	}

}