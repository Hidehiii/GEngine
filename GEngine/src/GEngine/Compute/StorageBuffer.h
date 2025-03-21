#pragma once
#include "GEngine/Core/Core.h"

namespace GEngine
{
	class CommandBuffer;
	class GENGINE_API StorageBuffer
	{
	public:
		virtual void SetData(uint32_t size, const void* data, uint32_t offset = 0) = 0;
		virtual void Bind(uint32_t slot) = 0;

		static Ref<StorageBuffer> Create(uint32_t size);
	};
}


