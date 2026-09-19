#include "GEpch.h"
#include "GEngine/Compute/StorageBuffer.h"
#include "GEngine/Graphics/Graphics.h"

namespace GEngine
{
	Ref<StorageBuffer> StorageBuffer::Create(uint32_t size)
	{
		return Graphics::GetRenderDevice().CreateStorageBuffer(size);
	}
}
