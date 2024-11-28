#pragma once
#include <glad/glad.h>
#include "GEngine/Renderer/ImageFormat.h"
namespace GEngine
{
	namespace Utils
	{
		GLenum Image2DFormatToGLInternalFormat(Image2DFormat format);
		GLenum Image2DFormatToGLDataFormat(Image2DFormat format);
	}
}

