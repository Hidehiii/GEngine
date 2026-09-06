#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsResource.h"

namespace GEngine
{
	class CommandBuffer;
	class GENGINE_API StorageBuffer : public GraphicsResource
	{
	public:
		virtual ~StorageBuffer() = default;
		GraphicsResourceType GetResourceType() const override { return GraphicsResourceType::Buffer; }
		virtual void SetData(uint32_t size, const void* data, uint32_t offset = 0) = 0;
		virtual void Bind(uint32_t slot) = 0;

		static Ref<StorageBuffer> Create(uint32_t size);
	protected:
		void* GetNativeResource() const override { return nullptr; }
	};
}


