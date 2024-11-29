#include "GEpch.h"
#include "OpenGLStorageImage2D.h"
#include <glad/glad.h>
#include "OpenGLUtils.h"

namespace GEngine
{

	OpenGLStorageImage2D::OpenGLStorageImage2D(uint32_t width, uint32_t height, ComputeImage2DFormat format)
	{
		m_Width = width;
		m_Height = height;
		m_Format = format;
		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTexStorage2D(GL_TEXTURE_2D, 1, Utils::ComputeImage2DFormatToGLInternalFormat(format), width, height);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	OpenGLStorageImage2D::~OpenGLStorageImage2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}
	void OpenGLStorageImage2D::SetData(const void* data, uint32_t size)
	{
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, Utils::ComputeImage2DFormatToGLDataFormat(m_Format), GL_UNSIGNED_BYTE, data);
	}
	void OpenGLStorageImage2D::Bind(const uint32_t slot)
	{
		glBindImageTexture(slot, m_RendererID, 0, 0, 0, GL_READ_WRITE, Utils::ComputeImage2DFormatToGLInternalFormat(m_Format));
	}
}