#pragma once
#include "GEngine/Core/Core.h"

namespace GEngine
{
	enum class StorageImage2DFormat
	{
		R32U,
	};


	class GENGINE_API StorageImage2D
	{
	public:

		static Ref<StorageImage2D> Create(uint32_t width, uint32_t height, StorageImage2DFormat format);
	};
}


