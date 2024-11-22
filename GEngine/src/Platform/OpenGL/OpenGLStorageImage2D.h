#pragma once
#include "GEngine/Compute/StorageImage.h"

namespace GEngine
{
	class GENGINE_API OpenGLStorageImage2D : public StorageImage2D
	{
	public:
		OpenGLStorageImage2D(uint32_t width, uint32_t height, StorageImage2DFormat format);
	};
}


