#pragma once
#include <glad/glad.h>
#include "GEngine/Graphics/ImageFormat.h"
#include "GEngine/Graphics/Shader.h"
#include "GEngine/Graphics/Sampler.h"
namespace GEngine
{
	namespace Utils
	{
		GLenum RenderImage2DFormatToGLInternalFormat(RenderImage2DFormat format);
		GLenum RenderImage2DFormatToGLDataFormat(RenderImage2DFormat format);

		GLenum ComputeImage2DFormatToGLInternalFormat(ComputeImage2DFormat format);
		GLenum ComputeImage2DFormatToGLDataFormat(ComputeImage2DFormat format);

		GLenum BlendFactorToGLBlendFactor(BlendFactor factor);

		GLenum SamplerFilterToGLSamplerFilter(SamplerFilter filter, SamplerFilter mipmapMode);
		GLenum SamplerAddressModeToGLWrapMode(SamplerAddressMode mode);
	}
}

