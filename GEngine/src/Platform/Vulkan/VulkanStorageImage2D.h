#pragma once
#include "GEngine/Compute/StorageImage.h"

namespace GEngine
{
	class GENGINE_API VulkanStorageImage2D : public StorageImage2D
	{
	public:
		VulkanStorageImage2D(uint32_t width, uint32_t height, StorageImage2DFormat format);
	};
}


