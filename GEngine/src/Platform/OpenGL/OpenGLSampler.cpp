#include "GEpch.h"
#include "OpenGLSampler.h"
#include "OpenGLUtils.h"
#include <glad/glad.h>

namespace GEngine
{
    OpenGLSampler::OpenGLSampler(const SamplerSpecification& spec)
    {
        m_Specification = spec;
        glCreateSamplers(1, &m_Sampler);
        
        glSamplerParameteri(m_Sampler, GL_TEXTURE_MAG_FILTER, Utils::SamplerFilterToGLSamplerFilter(m_Specification.MagFilter, m_Specification.MipmapFilter));
        glSamplerParameteri(m_Sampler, GL_TEXTURE_MIN_FILTER, Utils::SamplerFilterToGLSamplerFilter(m_Specification.MinFilter, m_Specification.MipmapFilter));
        glSamplerParameteri(m_Sampler, GL_TEXTURE_WRAP_S, Utils::SamplerAddressModeToGLWrapMode(m_Specification.WrapU));
        glSamplerParameteri(m_Sampler, GL_TEXTURE_WRAP_T, Utils::SamplerAddressModeToGLWrapMode(m_Specification.WrapV));
        glSamplerParameteri(m_Sampler, GL_TEXTURE_WRAP_R, Utils::SamplerAddressModeToGLWrapMode(m_Specification.WrapW));

		GLfloat maxAniso = 0.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
        glSamplerParameterf(m_Sampler, GL_TEXTURE_MAX_ANISOTROPY, maxAniso);
    }
    OpenGLSampler::~OpenGLSampler()
    {
        glDeleteSamplers(1, &m_Sampler);
    }
    void OpenGLSampler::Bind(uint32_t slot)
    {
        glBindSampler(slot, m_Sampler);
    }
}