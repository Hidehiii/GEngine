#include "GEpch.h"
#include "OpenGLCubeMap.h"
#include "stb_image.h"
#include <glad/glad.h>
#include "OpenGLUtils.h"

namespace GEngine
{
	OpenGLCubeMap::OpenGLCubeMap(uint32_t width, uint32_t height, RenderImage2DFormat format)
	{
		m_Format = format;
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, Utils::RenderImage2DFormatToGLInternalFormat(format), m_Width, m_Height);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	OpenGLCubeMap::OpenGLCubeMap(const std::string& rightPath, const std::string& leftPath, const std::string& topPath, const std::string& buttomPath, const std::string& backPath, const std::string& frontPath)
	{
	}
	OpenGLCubeMap::~OpenGLCubeMap()
	{
		glDeleteTextures(1, &m_RendererID);
	}
}

