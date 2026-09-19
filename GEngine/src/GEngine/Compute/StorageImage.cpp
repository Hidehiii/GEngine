#include "GEpch.h"
#include "GEngine/Compute/StorageImage.h"
#include "GEngine/Graphics/Graphics.h"
namespace GEngine
{
	Ref<StorageImage2D> StorageImage2D::Create(uint32_t width, uint32_t height, ComputeImage2DFormat format)
	{
		return Graphics::GetRenderDevice().CreateStorageImage2D(width, height, format);
	}
}
