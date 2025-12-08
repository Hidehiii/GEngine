#include "GEpch.h"
#include "OpenGLTexture2DCombineSampler.h"

namespace GEngine
{
    OpenGLTexture2DCombineSampler::OpenGLTexture2DCombineSampler(const Ref<Texture2D>& texture, const Ref<Sampler>& sampler)
    {
        m_Texture = std::dynamic_pointer_cast<OpenGLTexture2D>(texture);
		m_Sampler = std::dynamic_pointer_cast<OpenGLSampler>(sampler);
    }

    void OpenGLTexture2DCombineSampler::Bind(CommandBuffer* cmdBuffer, const uint32_t slot)
    {
		glBindTexture(GL_TEXTURE_2D + slot, m_Texture->GetOpenGLID());
		m_Sampler->Bind(slot);
    }

    void OpenGLTexture2DCombineSampler::SetTexture(const Ref<Texture2D>& texture)
    {
		m_Texture = std::dynamic_pointer_cast<OpenGLTexture2D>(texture);
    }

    void OpenGLTexture2DCombineSampler::SetSampler(const Ref<Sampler>& sampler)
    {
		m_Sampler = std::dynamic_pointer_cast<OpenGLSampler>(sampler);
    }


}