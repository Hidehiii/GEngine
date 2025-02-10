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
			case ComputeImage2DFormat::RGBA32F:		return GL_RGBA32F;
			case ComputeImage2DFormat::RGBA16F:		return GL_RGBA16F;
			case ComputeImage2DFormat::RG32F:		return GL_RG32F;
			case ComputeImage2DFormat::RG16F:		return GL_RG16F;
			case ComputeImage2DFormat::R32F:		return GL_R32F;
			case ComputeImage2DFormat::R16F:		return GL_R16F;
			case ComputeImage2DFormat::RGBA32I:		return GL_RGBA32I;
			case ComputeImage2DFormat::RGBA16I:		return GL_RGBA16I;
			case ComputeImage2DFormat::RGBA8I:		return GL_RGBA8I;
			case ComputeImage2DFormat::RGBA32UI:	return GL_RGBA32UI;
			case ComputeImage2DFormat::RGBA16UI:	return GL_RGBA16UI;
			case ComputeImage2DFormat::RGBA8UI:		return GL_RGBA8UI;
			case ComputeImage2DFormat::R32UI:		return GL_R32UI;
			default:
				break;
			}
			return GLenum();
		}
		GLenum ComputeImage2DFormatToGLDataFormat(ComputeImage2DFormat format)
		{
			switch (format)
			{
			case ComputeImage2DFormat::RGBA32F:		return GL_RGBA;
			case ComputeImage2DFormat::RGBA16F:		return GL_RGBA;
			case ComputeImage2DFormat::RG32F:		return GL_RG;
			case ComputeImage2DFormat::RG16F:		return GL_RG;
			case ComputeImage2DFormat::R32F:		return GL_R;
			case ComputeImage2DFormat::R16F:		return GL_R;
			case ComputeImage2DFormat::RGBA32I:		return GL_RGBA;
			case ComputeImage2DFormat::RGBA16I:		return GL_RGBA;
			case ComputeImage2DFormat::RGBA8I:		return GL_RGBA;
			case ComputeImage2DFormat::RGBA32UI:	return GL_RGBA;
			case ComputeImage2DFormat::RGBA16UI:	return GL_RGBA;
			case ComputeImage2DFormat::RGBA8UI:		return GL_RGBA;
			case ComputeImage2DFormat::R32UI:		return GL_R;
				break;
			default:
				break;
			}
			return GLenum();
		}
		GLenum BlendFactorToGLBlendFactor(BlendFactor factor)
		{
			switch (factor)
			{
			case BlendFactor::SRC_ALPHA: return GL_SRC_ALPHA;
			case BlendFactor::DST_ALPHA: return GL_DST_ALPHA;
			case BlendFactor::SRC_COLOR: return GL_SRC_COLOR;
			case BlendFactor::DST_COLOR: return GL_DST_COLOR;
			case BlendFactor::ONE_MINUS_SRC_ALPHA: return GL_ONE_MINUS_SRC_ALPHA;
			case BlendFactor::ONE_MINUS_DST_ALPHA: return GL_ONE_MINUS_DST_ALPHA;
			case BlendFactor::ONE_MINUS_SRC_COLOR: return GL_ONE_MINUS_SRC_COLOR;
			case BlendFactor::ONE_MINUS_DST_COLOR: return GL_ONE_MINUS_DST_COLOR;
			case BlendFactor::ONE: return GL_ONE;
			case BlendFactor::ZERO: return GL_ZERO;
			default:
				break;
			}
			return GLenum();
		}
		GLenum SamplerFilterToGLSamplerFilter(SamplerFilter filter, SamplerFilter mipmapMode)
		{
			if (filter == SamplerFilter::Liner && mipmapMode == SamplerFilter::Liner)
			{
				return GL_LINEAR_MIPMAP_LINEAR;
			}
			else if (filter == SamplerFilter::Liner && mipmapMode == SamplerFilter::Nearest)
			{
				return GL_LINEAR_MIPMAP_NEAREST;
			}
			else if (filter == SamplerFilter::Nearest && mipmapMode == SamplerFilter::Liner)
			{
				return GL_NEAREST_MIPMAP_LINEAR;
			}
			else if (filter == SamplerFilter::Nearest && mipmapMode == SamplerFilter::Nearest)
			{
				return GL_NEAREST_MIPMAP_NEAREST;
			}
			return GL_LINEAR_MIPMAP_LINEAR;
		}
		GLenum SamplerAddressModeToGLWrapMode(SamplerAddressMode mode)
		{
			switch (mode)
			{
			case SamplerAddressMode::Repeat:
				return GL_REPEAT;
			case SamplerAddressMode::MirroredRepeat:
				return GL_MIRRORED_REPEAT;
			case SamplerAddressMode::ClampToEdge:
				return GL_CLAMP_TO_EDGE;
			case SamplerAddressMode::ClampToBorder:
				return GL_CLAMP_TO_BORDER;
			case SamplerAddressMode::MirrorClampToEdge:
				return GL_MIRROR_CLAMP_TO_EDGE;
			default:
				break;
			}
			return GL_REPEAT;
		}
	}
}