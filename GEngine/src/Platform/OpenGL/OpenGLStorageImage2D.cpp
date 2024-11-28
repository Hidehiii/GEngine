#include "GEpch.h"
#include "OpenGLStorageImage2D.h"
#include <glad/glad.h>
#include "OpenGLUtils.h"

namespace GEngine
{

	OpenGLStorageImage2D::OpenGLStorageImage2D(uint32_t width, uint32_t height, Image2DFormat format)
	{
		m_Width = width;
		m_Height = height;
		m_Format = format;
		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTexStorage2D(GL_TEXTURE_2D, 1, Utils::Image2DFormatToGLInternalFormat(format), width, height);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}