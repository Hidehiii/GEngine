#include "GEpch.h"
#include "OpenGLTexture2DArray.h"
#include <glad/glad.h>
#include "OpenGLUtils.h"
#include "Platform/OpenGL/OpenGLTexture2D.h"

namespace GEngine
{
    OpenGLTexture2DArray::OpenGLTexture2DArray(uint32_t width, uint32_t height, uint32_t layers, RenderImage2DFormat format)
    {
        m_Format    = format;
        m_Width     = width;
        m_Height    = height;
        m_Layers    = layers;
		if (m_GenerateMipmap)
		{
			m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;
		}

        glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_TextureArray);
        glTextureStorage3D(m_TextureArray, m_MipLevels, Utils::RenderImage2DFormatToGLInternalFormat(m_Format), m_Width, m_Height, m_Layers);
		glTextureParameteri(m_TextureArray, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_TextureArray, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_TextureArray, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTextureParameteri(m_TextureArray, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_TextureArray, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    OpenGLTexture2DArray::~OpenGLTexture2DArray()
    {
        glDeleteTextures(1, &m_TextureArray);
    }
    void OpenGLTexture2DArray::Bind(const uint32_t slot)
    {
        glBindTextureUnit(slot, m_TextureArray);
    }
    void OpenGLTexture2DArray::SetData(const void* data, uint32_t size)
    {
        uint32_t tempSize = m_Width * m_Height * RenderImage2DFormatChannelSize(m_Format);
        for (uint32_t layer = 0; layer < size / tempSize; layer++)
        {
            uint32_t offset = tempSize * layer;
			void* tempData = new std::byte[tempSize];
			memcpy(tempData, ((std::byte*)data) + offset, tempSize);
			SetData(tempData, tempSize, layer);
			delete[](std::byte*)tempData;
        }
    }
    void OpenGLTexture2DArray::SetData(const Ref<Texture2D>& texture, uint32_t width, uint32_t height, uint32_t layer)
    {
		GLint currentFbo;
		GLuint fbo;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFbo);
		glCreateFramebuffers(1, &fbo);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, std::dynamic_pointer_cast<OpenGLTexture2D>(texture)->GetOpenGLID(), 0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_TextureArray);
		glCopyTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, 0, 0, width, height);
		glBindFramebuffer(GL_FRAMEBUFFER, currentFbo);
		glDeleteFramebuffers(1, &fbo);
		if (m_GenerateMipmap)
		{
			glGenerateTextureMipmap(m_TextureArray);
		}
    }
    void OpenGLTexture2DArray::SetData(const void* data, uint32_t size, uint32_t layer)
    {
        glTextureSubImage3D(m_TextureArray, 0, 0, 0, layer, m_Width, m_Height, 1,
            Utils::RenderImage2DFormatToGLDataFormat(m_Format), GL_UNSIGNED_BYTE, data); 
		if (m_GenerateMipmap)
		{
			glGenerateTextureMipmap(m_TextureArray);
		}
    }
}