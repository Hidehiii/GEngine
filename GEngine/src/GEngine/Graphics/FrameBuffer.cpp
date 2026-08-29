#include "GEpch.h"
#include "FrameBuffer.h"
#include "GEngine/Graphics/Graphics.h"

namespace GEngine
{

	Ref<FrameBuffer> FrameBuffer::Create(const Ref<RenderPass>& renderPass, const Vector2& size)
	{
		return Create(renderPass, size.x, size.y);
	}

	Ref<FrameBuffer> FrameBuffer::Create(const Ref<RenderPass>& renderPass, uint32_t width, uint32_t height)
	{
		return Graphics::GetRenderDevice().CreateFrameBuffer(renderPass, width, height);
	}

	Ref<FrameBuffer> FrameBuffer::Create(const Ref<FrameBuffer>& buffer, uint32_t width, uint32_t height)
	{
		return Graphics::GetRenderDevice().ResizeFrameBuffer(buffer, width, height);
	}

	Ref<FrameBuffer> FrameBuffer::Create(const Ref<FrameBuffer>& buffer, Vector2 size)
	{
		return Create(buffer, size.x, size.y);
	}
}
