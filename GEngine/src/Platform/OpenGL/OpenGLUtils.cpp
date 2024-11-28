#include "GEpch.h"
#include "OpenGLUtils.h"


namespace GEngine
{
	namespace Utils
	{
		GLenum Image2DFormatToGLInternalFormat(Image2DFormat format)
		{
			switch (format)
			{
			case GEngine::Image2DFormat::R8G8B8A8F:	return GL_RGBA8;
			case GEngine::Image2DFormat::R8G8B8F:	return GL_RGB8;
			case GEngine::Image2DFormat::R32UI:		return GL_R32UI;
			default:
				break;
			}
			return GL_RGBA8;
		}
		GLenum Image2DFormatToGLDataFormat(Image2DFormat format)
		{
			switch (format)
			{
			case GEngine::Image2DFormat::R8G8B8A8F:	return GL_RGBA;
			case GEngine::Image2DFormat::R8G8B8F:	return GL_RGB;
			case GEngine::Image2DFormat::R32UI:		return GL_R;
			default:
				break;
			}
		}
	}
}