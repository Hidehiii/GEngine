#pragma once
#include "GEngine/Compute/StorageImage.h"

namespace GEngine
{
	class GENGINE_API OpenGLStorageImage2D : public StorageImage2D
	{
	public:
		OpenGLStorageImage2D(uint32_t width, uint32_t height, ComputeImage2DFormat format);
		virtual ~OpenGLStorageImage2D();

		virtual void Bind(const uint32_t slot = 0) override;
	private:
		uint32_t	m_Image = 0;
		uint32_t			m_Width, m_Height;
		ComputeImage2DFormat	m_Format;
	};
}


