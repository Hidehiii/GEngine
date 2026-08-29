#include "GEpch.h"
#include "UniformBuffer.h"
#include "GEngine/Graphics/Graphics.h"

namespace GEngine
{
	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t count, bool autoSetDataDynamic)
	{
		return Graphics::GetRenderDevice().CreateUniformBuffer(size, count, autoSetDataDynamic);
	}
}
