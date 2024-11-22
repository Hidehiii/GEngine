#pragma once
#include "GEngine/Core/Core.h"

namespace GEngine
{
	class GENGINE_API StorageBuffer
	{
	public:
		virtual void SetData(uint32_t size, const void* data, uint32_t offset = 0) = 0;

		static Ref<StorageBuffer> Create(uint32_t size, uint32_t binding);
	};
}


