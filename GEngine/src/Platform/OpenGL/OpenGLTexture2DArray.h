#pragma once
#include "GEngine/Renderer/Texture.h"

namespace GEngine
{
	class GENGINE_API OpenGLTexture2DArray : public Texture2DArray
	{
	public:
		OpenGLTexture2DArray(uint32_t width, uint32_t height, uint32_t layers, RenderImage2DFormat format);
	};
}


