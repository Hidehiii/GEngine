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
			case RENDER_IMAGE_2D_FORMAT_RGBA8F:	return GL_RGBA8;
			case RENDER_IMAGE_2D_FORMAT_RGB8F:	return GL_RGB8;
			default:
				break;
			}
			return GL_RGBA8;
		}
		GLenum RenderImage2DFormatToGLDataFormat(RenderImage2DFormat format)
		{
			switch (format)
			{
			case RENDER_IMAGE_2D_FORMAT_RGBA8F:	return GL_RGBA;
			case RENDER_IMAGE_2D_FORMAT_RGB8F:	return GL_RGB;
			default:
				break;
			}
		}
		GLenum ComputeImage2DFormatToGLInternalFormat(ComputeImage2DFormat format)
		{
			switch (format)
			{
			case COMPUTE_IMAGE_2D_FORMAT_RGBA32F:		return GL_RGBA32F;
			case COMPUTE_IMAGE_2D_FORMAT_RGBA16F:		return GL_RGBA16F;
			case COMPUTE_IMAGE_2D_FORMAT_RG32F:			return GL_RG32F;
			case COMPUTE_IMAGE_2D_FORMAT_RG16F:			return GL_RG16F;
			case COMPUTE_IMAGE_2D_FORMAT_R32F:			return GL_R32F;
			case COMPUTE_IMAGE_2D_FORMAT_R16F:			return GL_R16F;
			case COMPUTE_IMAGE_2D_FORMAT_RGBA32I:		return GL_RGBA32I;
			case COMPUTE_IMAGE_2D_FORMAT_RGBA16I:		return GL_RGBA16I;
			case COMPUTE_IMAGE_2D_FORMAT_RGBA8I:		return GL_RGBA8I;
			case COMPUTE_IMAGE_2D_FORMAT_RGBA32UI:		return GL_RGBA32UI;
			case COMPUTE_IMAGE_2D_FORMAT_RGBA16UI:		return GL_RGBA16UI;
			case COMPUTE_IMAGE_2D_FORMAT_RGBA8UI:		return GL_RGBA8UI;
			case COMPUTE_IMAGE_2D_FORMAT_R32UI:			return GL_R32UI;
			default:
				break;
			}
			return GLenum();
		}
		GLenum ComputeImage2DFormatToGLDataFormat(ComputeImage2DFormat format)
		{
			switch (format)
			{
			case COMPUTE_IMAGE_2D_FORMAT_RGBA32F:		return GL_RGBA;
			case COMPUTE_IMAGE_2D_FORMAT_RGBA16F:		return GL_RGBA;
			case COMPUTE_IMAGE_2D_FORMAT_RG32F:			return GL_RG;
			case COMPUTE_IMAGE_2D_FORMAT_RG16F:			return GL_RG;
			case COMPUTE_IMAGE_2D_FORMAT_R32F:			return GL_R;
			case COMPUTE_IMAGE_2D_FORMAT_R16F:			return GL_R;
			case COMPUTE_IMAGE_2D_FORMAT_RGBA32I:		return GL_RGBA;
			case COMPUTE_IMAGE_2D_FORMAT_RGBA16I:		return GL_RGBA;
			case COMPUTE_IMAGE_2D_FORMAT_RGBA8I:		return GL_RGBA;
			case COMPUTE_IMAGE_2D_FORMAT_RGBA32UI:		return GL_RGBA;
			case COMPUTE_IMAGE_2D_FORMAT_RGBA16UI:		return GL_RGBA;
			case COMPUTE_IMAGE_2D_FORMAT_RGBA8UI:		return GL_RGBA;
			case COMPUTE_IMAGE_2D_FORMAT_R32UI:			return GL_R;
				break;
			default:
				break;
			}
			return GLenum();
		}
		uint32_t FrameBufferTextureFormatToGLInternalFormat(FrameBufferTextureFormat format)
		{
			switch (format)
			{
			case FRAME_BUFFER_TEXTURE_FORMAT_NONE:
				break;
			case FRAME_BUFFER_TEXTURE_FORMAT_RGBA8:
				return GL_RGBA8;
			case FRAME_BUFFER_TEXTURE_FORMAT_R32F:
				return GL_R32F;
			case FRAME_BUFFER_TEXTURE_FORMAT_RG16F:
				return GL_RG16F;
			case FRAME_BUFFER_TEXTURE_FORMAT_R32I:
				return GL_R32I;
			case FRAME_BUFFER_TEXTURE_FORMAT_RG16I:
				return GL_RG16I;
			case FRAME_BUFFER_TEXTURE_FORMAT_R32UI:
				return GL_R32UI;
			case FRAME_BUFFER_TEXTURE_FORMAT_RG16UI:
				return GL_RG16UI;
			case FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8:
				return GL_DEPTH24_STENCIL8;
			case FRAME_BUFFER_TEXTURE_FORMAT_DEPTH:
				return GL_DEPTH_COMPONENT32F;
			default:
				break;
			}
			return uint32_t();
		}
		uint32_t FrameBufferTextureFormatToGLDataFormat(FrameBufferTextureFormat format)
		{
			switch (format)
			{
			case FRAME_BUFFER_TEXTURE_FORMAT_NONE:
				break;
			case FRAME_BUFFER_TEXTURE_FORMAT_RGBA8:
				return GL_RGBA;
			case FRAME_BUFFER_TEXTURE_FORMAT_R32F:
				return GL_R;
			case FRAME_BUFFER_TEXTURE_FORMAT_RG16F:
				return GL_RG;
			case FRAME_BUFFER_TEXTURE_FORMAT_R32I:
				return GL_R;
			case FRAME_BUFFER_TEXTURE_FORMAT_RG16I:
				return GL_RG;
			case FRAME_BUFFER_TEXTURE_FORMAT_R32UI:
				return GL_R;
			case FRAME_BUFFER_TEXTURE_FORMAT_RG16UI:
				return GL_RG;
			case FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8:
				return GL_DEPTH_STENCIL_ATTACHMENT;
			case FRAME_BUFFER_TEXTURE_FORMAT_DEPTH:
				return GL_DEPTH_ATTACHMENT;
			default:
				break;
			}
			return uint32_t();
		}
		GLenum BlendFactorToGLBlendFactor(BlendFactor factor)
		{
			switch (factor)
			{
			case BLEND_FACTOR_SRC_ALPHA:			return GL_SRC_ALPHA;
			case BLEND_FACTOR_DST_ALPHA:			return GL_DST_ALPHA;
			case BLEND_FACTOR_SRC_COLOR:			return GL_SRC_COLOR;
			case BLEND_FACTOR_DST_COLOR:			return GL_DST_COLOR;
			case BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:	return GL_ONE_MINUS_SRC_ALPHA;
			case BLEND_FACTOR_ONE_MINUS_DST_ALPHA:	return GL_ONE_MINUS_DST_ALPHA;
			case BLEND_FACTOR_ONE_MINUS_SRC_COLOR:	return GL_ONE_MINUS_SRC_COLOR;
			case BLEND_FACTOR_ONE_MINUS_DST_COLOR:	return GL_ONE_MINUS_DST_COLOR;
			case BLEND_FACTOR_ONE:					return GL_ONE;
			case BLEND_FACTOR_ZERO:					return GL_ZERO;
			default:
				break;
			}
			return GLenum();
		}
		GLenum SamplerFilterToGLSamplerFilter(SamplerFilter filter, SamplerFilter mipmapMode)
		{
			if (filter == SAMPLER_FILTER_LINEAR && mipmapMode == SAMPLER_FILTER_LINEAR)
			{
				return GL_LINEAR_MIPMAP_LINEAR;
			}
			else if (filter == SAMPLER_FILTER_LINEAR && mipmapMode == SAMPLER_FILTER_NEAREST)
			{
				return GL_LINEAR_MIPMAP_NEAREST;
			}
			else if (filter == SAMPLER_FILTER_NEAREST && mipmapMode == SAMPLER_FILTER_LINEAR)
			{
				return GL_NEAREST_MIPMAP_LINEAR;
			}
			else if (filter == SAMPLER_FILTER_NEAREST && mipmapMode == SAMPLER_FILTER_NEAREST)
			{
				return GL_NEAREST_MIPMAP_NEAREST;
			}
			return GL_LINEAR_MIPMAP_LINEAR;
		}
		GLenum SamplerAddressModeToGLWrapMode(SamplerAddressMode mode)
		{
			switch (mode)
			{
			case SAMPLER_ADDRESS_REPEAT:
				return GL_REPEAT;
			case SAMPLER_ADDRESS_MIRRORED_REPEAT:
				return GL_MIRRORED_REPEAT;
			case SAMPLER_ADDRESS_CLAMP_TO_EDGE:
				return GL_CLAMP_TO_EDGE;
			case SAMPLER_ADDRESS_CLAMP_TO_BORDER:
				return GL_CLAMP_TO_BORDER;
			case SAMPLER_ADDRESS_MIRROR_CLAMP_TO_EDGE:
				return GL_MIRROR_CLAMP_TO_EDGE;
			default:
				break;
			}
			return GL_REPEAT;
		}
		GLenum ShaderInputDataTypeToGlDataType(ShaderInputDataType type)
		{
			switch (type)
			{
			case ShaderDataType::float1: return GL_FLOAT;
			case ShaderDataType::float2: return GL_FLOAT;
			case ShaderDataType::float3: return GL_FLOAT;
			case ShaderDataType::float4: return GL_FLOAT;
			case ShaderDataType::int1:   return GL_INT;
			case ShaderDataType::int2:   return GL_INT;
			case ShaderDataType::int3:   return GL_INT;
			case ShaderDataType::int4:   return GL_INT;
			case ShaderDataType::mat3:   return GL_FLOAT;
			case ShaderDataType::mat4:   return GL_FLOAT;
			}
			GE_CORE_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}
		void EnableDepthWrite(bool enable)
		{
			if (enable)
				glDepthMask(GL_TRUE);
			else
				glDepthMask(GL_FALSE);
		}
		void SetDepthTest(CompareOperation op)
		{
			switch (op)
			{
			case COMPARE_OP_LESS:			glDepthFunc(GL_LESS); break;
			case COMPARE_OP_GREATER:		glDepthFunc(GL_GREATER); break;
			case COMPARE_OP_LESS_EQUAL:		glDepthFunc(GL_LEQUAL); break;
			case COMPARE_OP_GREATER_EQUAL:	glDepthFunc(GL_GEQUAL); break;
			case COMPARE_OP_EQUAL:			glDepthFunc(GL_EQUAL); break;
			case COMPARE_OP_NOT_EQUAL:		glDepthFunc(GL_NOTEQUAL); break;
			case COMPARE_OP_ALWAYS:			glDepthFunc(GL_ALWAYS); break;
			default:
				break;
			}
		}
		void SetCull(CullMode mode)
		{
			switch (mode)
			{
			case CULL_MODE_NONE:		glDisable(GL_CULL_FACE); break;
			case CULL_MODE_BACK:		glEnable(GL_CULL_FACE); glCullFace(GL_BACK); break;
			case CULL_MODE_FRONT:		glEnable(GL_CULL_FACE); glCullFace(GL_FRONT); break;
			default:				GE_CORE_CRITICAL("Unknown cull mode!"); break;
			}
		}
		void SetBlend(BlendMode modeColor, BlendMode modeAlpha, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha)
		{
			GLenum colorMode = GL_FUNC_ADD, alphaMode = GL_FUNC_ADD;
			switch (modeColor)
			{
			case BLEND_MODE_NONE:
				glDisable(GL_BLEND);
				return;
			case BLEND_MODE_ADD:
				colorMode = GL_FUNC_ADD;
				break;
			case BLEND_MODE_SUBSTRACT:
				colorMode = (GL_FUNC_SUBTRACT);
				break;
			case BLEND_MODE_REVERSE_SUBSTRACT:
				colorMode = (GL_FUNC_REVERSE_SUBTRACT);
				break;
			case BLEND_MODE_MIN:
				colorMode = (GL_MIN);
				break;
			case BLEND_MODE_MAX:
				colorMode = (GL_MAX);
				break;
			default:
				break;
			}
			switch (modeAlpha)
			{
			case BLEND_MODE_NONE:
				glDisable(GL_BLEND);
				return;
			case BLEND_MODE_ADD:
				alphaMode = GL_FUNC_ADD;
				break;
			case BLEND_MODE_SUBSTRACT:
				alphaMode = GL_FUNC_SUBTRACT;
				break;
			case BLEND_MODE_REVERSE_SUBSTRACT:
				alphaMode = GL_FUNC_REVERSE_SUBTRACT;
				break;
			case BLEND_MODE_MIN:
				alphaMode = GL_MIN;
				break;
			case BLEND_MODE_MAX:
				alphaMode = GL_MAX;
				break;
			default:
				break;
			}
			glBlendEquationSeparate(colorMode, alphaMode);
			glBlendFuncSeparate(Utils::BlendFactorToGLBlendFactor(srcColor),
				Utils::BlendFactorToGLBlendFactor(dstColor),
				Utils::BlendFactorToGLBlendFactor(srcAlpha),
				Utils::BlendFactorToGLBlendFactor(dstAlpha));
		}
	}
}