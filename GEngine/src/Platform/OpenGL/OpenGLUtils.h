#pragma once
#include <glad/glad.h>
#include "GEngine/Renderer/ImageFormat.h"
#include "GEngine/Renderer/Shader.h"
#include "GEngine/Renderer/Sampler.h"
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

