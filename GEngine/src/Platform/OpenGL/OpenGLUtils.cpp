#include "GEpch.h"
#include "OpenGLUtils.h"


namespace GEngine
{
	namespace Utils
	{
		GLenum RenderImage2DFormatToGLInternalFormat(RenderImage2DFormat format)
		{
			switch (format)
			{
			case RenderImage2DFormat::RGBA8F:	return GL_RGBA8;
			case RenderImage2DFormat::RGB8F:	return GL_RGB8;
			default:
				break;
			}
			return GL_RGBA8;
		}
		GLenum RenderImage2DFormatToGLDataFormat(RenderImage2DFormat format)
		{
			switch (format)
			{
			case RenderImage2DFormat::RGBA8F:	return GL_RGBA;
			case RenderImage2DFormat::RGB8F:	return GL_RGB;
			default:
				break;
			}
		}
		GLenum ComputeImage2DFormatToGLInternalFormat(ComputeImage2DFormat format)
		{
			switch (format)
			{
			case ComputeImage2DFormat::RGBA32F:	return GL_RGBA32F;
			case ComputeImage2DFormat::RGBA16F:	return GL_RGBA16F;
			case ComputeImage2DFormat::RG32F:		return GL_RG32F;
			case ComputeImage2DFormat::RG16F:		return GL_RG16F;
			case ComputeImage2DFormat::R32F:		return GL_R32F;
			case ComputeImage2DFormat::R16F:		return GL_R16F;
			case ComputeImage2DFormat::RGBA32I:	return GL_RGBA32I;
			case ComputeImage2DFormat::RGBA16I:	return GL_RGBA16I;
			case ComputeImage2DFormat::RGBA8I:		return GL_RGBA8I;
			case ComputeImage2DFormat::RGBA32UI:	return GL_RGBA32UI;
			case ComputeImage2DFormat::RGBA16UI:	return GL_RGBA16UI;
			case ComputeImage2DFormat::RGBA8UI:	return GL_RGBA8UI;
			default:
				break;
			}
			return GLenum();
		}
		GLenum ComputeImage2DFormatToGLDataFormat(ComputeImage2DFormat format)
		{
			switch (format)
			{
			case ComputeImage2DFormat::RGBA32F:	return GL_RGBA;
			case ComputeImage2DFormat::RGBA16F:	return GL_RGBA;
			case ComputeImage2DFormat::RG32F:		return GL_RG;
			case ComputeImage2DFormat::RG16F:	return GL_RG;
			case ComputeImage2DFormat::R32F:	return GL_R;
			case ComputeImage2DFormat::R16F:	return GL_R;
			case ComputeImage2DFormat::RGBA32I: return GL_RGBA;
			case ComputeImage2DFormat::RGBA16I:return GL_RGBA;
			case ComputeImage2DFormat::RGBA8I:return GL_RGBA;
			case ComputeImage2DFormat::RGBA32UI:return GL_RGBA;
			case ComputeImage2DFormat::RGBA16UI:return GL_RGBA;
			case ComputeImage2DFormat::RGBA8UI:return GL_RGBA;
				break;
			default:
				break;
			}
			return GLenum();
		}
	}
}