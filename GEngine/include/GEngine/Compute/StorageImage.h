#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/ImageFormat.h"
#include "GEngine/Graphics/GraphicsResource.h"

namespace GEngine
{
	
	class CommandBuffer;

	class GENGINE_API StorageImage2D : public GraphicsResource
	{
	public:
		virtual ~StorageImage2D() = default;
		GraphicsResourceType GetResourceType() const override { return GraphicsResourceType::Texture; }
		virtual void Bind(CommandBuffer* cmdBuffer, const uint32_t slot = 0) = 0;

		static Ref<StorageImage2D> Create(uint32_t width, uint32_t height, ComputeImage2DFormat format);
	protected:
		void* GetNativeResource() const override { return nullptr; }
	};
}


