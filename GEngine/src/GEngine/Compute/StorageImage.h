#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/ImageFormat.h"

namespace GEngine
{
	


	class GENGINE_API StorageImage2D
	{
	public:

		static Ref<StorageImage2D> Create(uint32_t width, uint32_t height, Image2DFormat format);
	};
}


