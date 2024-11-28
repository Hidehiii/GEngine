#pragma once
#include "GEngine/Compute/StorageImage.h"

namespace GEngine
{
	class GENGINE_API OpenGLStorageImage2D : public StorageImage2D
	{
	public:
		OpenGLStorageImage2D(uint32_t width, uint32_t height, Image2DFormat format);
	private:
		uint32_t	m_RendererID = 0;
		uint32_t			m_Width, m_Height;
		Image2DFormat	m_Format;
	};
}


