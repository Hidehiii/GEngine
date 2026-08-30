#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsAPI.h"

#include <optional>
#include <vulkan/vulkan.h>

namespace GEngine
{
	

	class GENGINE_API VulkanGraphicsAPI : public GraphicsAPI
	{
	public:
		VulkanGraphicsAPI();
		virtual ~VulkanGraphicsAPI();

		virtual Ref<CommandBuffer> GetGraphicsCommandBuffer() override;
		virtual Ref<CommandBuffer> GetComputeCommandBuffer() override;

		virtual std::vector<std::string> GetExtensions() override;
		virtual GraphicsCapabilities GetCapabilities() const override;
		virtual Ref<RenderPass> CreateRenderPass(const RenderPassSpecification& spec) override;
		virtual Ref<FrameBuffer> CreateFrameBuffer(const Ref<RenderPass>& renderPass, uint32_t width, uint32_t height) override;
		virtual Ref<FrameBuffer> ResizeFrameBuffer(const Ref<FrameBuffer>& buffer, uint32_t width, uint32_t height) override;
		virtual Ref<Texture2D> CreateTexture2D(uint32_t width, uint32_t height, RenderImage2DFormat format) override;
		virtual Ref<Texture2D> CreateTexture2D(const std::string& path) override;
		virtual Ref<Texture2D> CreateTexture2D(uint32_t width, uint32_t height, void* data, uint32_t size, RenderImage2DFormat format) override;
		virtual Ref<UniformBuffer> CreateUniformBuffer(uint32_t size, uint32_t count, bool autoSetDataDynamic) override;
		virtual Ref<VertexBuffer> CreateVertexBuffer(uint32_t size, uint32_t sizeInstance, VertexTopology type) override;
		virtual Ref<VertexBuffer> CreateVertexBuffer(const void* vertices, uint32_t size, uint32_t sizeInstance, VertexTopology type) override;
		virtual Ref<IndexBuffer> CreateIndexBuffer(const uint32_t* indices, uint32_t count) override;

		virtual uint32_t GetMaxTexture2DSize() override;
		virtual uint32_t GetMaxCombinedTextureCount() override;
		virtual uint32_t GetMaxPerStageTextureCount() override;
		virtual uint32_t GetMaxTextureArrayLayers() override;

		virtual uint32_t GetMinUniformBufferOffsetAlignment() override;
		virtual uint32_t GetMaxUniformBufferSize() override;

		// compute
		virtual Vector3 GetMaxComputeWorkGroupCount() override;
		virtual Vector3 GetMaxComputeWorkGroupSize() override;
		virtual uint32_t GetMaxComputeWorkGroupInvocations() override;

		virtual void SetCommandsBarrier(Ref<CommandBuffer>& first, Ref<CommandBuffer>& second) override;
		virtual void TransitionResource(const Ref<CommandBuffer>& commandBuffer, void* nativeResource,
			GraphicsResourceType resourceType, GraphicsResourceState before, GraphicsResourceState after) override;
	};
}


