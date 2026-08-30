#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsCommon.h"

namespace GEngine
{
	class RenderPass;
	class FrameBuffer;
	class Texture2D;
	class UniformBuffer;
	class VertexBuffer;
	class IndexBuffer;

	class GENGINE_API RenderDevice
	{
	public:
		virtual ~RenderDevice() = default;

		virtual Ref<RenderPass> CreateRenderPass(const RenderPassSpecification& spec) = 0;
		virtual Ref<FrameBuffer> CreateFrameBuffer(const Ref<RenderPass>& renderPass, uint32_t width, uint32_t height) = 0;
		virtual Ref<FrameBuffer> ResizeFrameBuffer(const Ref<FrameBuffer>& buffer, uint32_t width, uint32_t height) = 0;

		virtual Ref<Texture2D> CreateTexture2D(uint32_t width, uint32_t height, RenderImage2DFormat format) = 0;
		virtual Ref<Texture2D> CreateTexture2D(const std::string& path) = 0;
		virtual Ref<Texture2D> CreateTexture2D(uint32_t width, uint32_t height, void* data, uint32_t size, RenderImage2DFormat format) = 0;

		virtual Ref<UniformBuffer> CreateUniformBuffer(uint32_t size, uint32_t count, bool autoSetDataDynamic) = 0;
		virtual Ref<VertexBuffer> CreateVertexBuffer(uint32_t size, uint32_t sizeInstance, VertexTopology type) = 0;
		virtual Ref<VertexBuffer> CreateVertexBuffer(const void* vertices, uint32_t size, uint32_t sizeInstance, VertexTopology type) = 0;
		virtual Ref<IndexBuffer> CreateIndexBuffer(const uint32_t* indices, uint32_t count) = 0;
	};
}
