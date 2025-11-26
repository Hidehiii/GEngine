#pragma once
#include "GEngine/Graphics/ImageFormat.h"
#include "GEngine/Graphics/Shader.h"
#include "GEngine/Graphics/Sampler.h"
#include "GEngine/Graphics/FrameBuffer.h"

#include <glad/glad.h>

namespace GEngine
{
	namespace Utils
	{
		uint32_t	RenderImage2DFormatToGLInternalFormat(RenderImage2DFormat format);
		uint32_t	RenderImage2DFormatToGLDataFormat(RenderImage2DFormat format);

		uint32_t	ComputeImage2DFormatToGLInternalFormat(ComputeImage2DFormat format);
		uint32_t	ComputeImage2DFormatToGLDataFormat(ComputeImage2DFormat format);

		uint32_t	FrameBufferTextureFormatToGLInternalFormat(FrameBufferTextureFormat format);
		uint32_t	FrameBufferTextureFormatToGLDataFormat(FrameBufferTextureFormat format);

		uint32_t	BlendFactorToGLBlendFactor(BlendFactor factor);

		uint32_t	SamplerFilterToGLSamplerFilter(SamplerFilter filter, SamplerFilter mipmapMode);
		uint32_t	SamplerAddressModeToGLWrapMode(SamplerAddressMode mode);

		GLenum		ShaderInputDataTypeToGlDataType(ShaderInputDataType type);

		void	EnableDepthWrite(bool enable);
		void	SetDepthTest(CompareOperation op);
		void	SetCull(CullMode mode);
		void	SetBlend(BlendMode modeColor, BlendMode modeAlpha, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha);
	}
}

