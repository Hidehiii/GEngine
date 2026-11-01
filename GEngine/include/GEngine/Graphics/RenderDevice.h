#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsCommon.h"
#include "GEngine/Graphics/GraphicsQueue.h"
#include "GEngine/Graphics/ImageFormat.h"
#include "GEngine/Graphics/GraphicsResource.h"
#include <stdexcept>

namespace GEngine
{
	class RenderPass;
	class FrameBuffer;
	class Texture2D;
	class UniformBuffer;
	class VertexBuffer;
	class IndexBuffer;
	class Texture2DCombineSampler;
	class CubeMapCombineSampler;
	class CubeMap;
	class Texture2DArray;
	class Shader;
	class Material;
	class GraphicsPipeline;
	class ComputePipeline;
	class Sampler;
	class StorageBuffer;
	class StorageImage2D;

	struct RenderTargetFormatCapabilities
	{
		bool ColorAttachment = false;
		bool DepthStencilAttachment = false;
		bool Sampled = false;
		bool ColorResolve = false;
		bool DepthResolve = false;
		uint32_t DepthResolveModes = 0;
		bool MultisampleLoad = false;
		uint32_t SampleCounts = 0;
		uint32_t MaxWidth = 0, MaxHeight = 0;

		bool SupportsSamples(uint32_t samples) const
		{
			return samples && (samples & (samples - 1)) == 0 && (SampleCounts & samples) != 0;
		}
		bool SupportsDepthResolve(DepthResolveMode mode) const
		{
			const auto bit = static_cast<uint32_t>(mode);
			return bit && (bit & (bit - 1)) == 0 && (DepthResolveModes & bit) != 0;
		}
	};

	class GENGINE_API RenderDevice
	{
	public:
		enum class QueueOwnershipKind { None, TransferImage, ComputeBuffer };
		virtual ~RenderDevice() = default;
		virtual GraphicsQueue& GetQueue(CommandBufferType type) = 0;
		virtual RenderTargetFormatCapabilities QueryRenderTargetFormat(FrameBufferTextureFormat format) const = 0;
		void ValidateRenderTarget(const RenderPassSpecification& spec, uint32_t width, uint32_t height) const
		{
			if (!width || !height || spec.RenderTargets.empty())
				throw std::invalid_argument("A render target requires dimensions and color attachments.");
			const auto& op = spec.Operation;
			if (op.ColorBegin < RENDER_PASS_BEGINE_OP_NONE || op.ColorBegin > RENDER_PASS_BEGINE_OP_CLEAR ||
				op.DepthStencilBegin < RENDER_PASS_BEGINE_OP_NONE || op.DepthStencilBegin > RENDER_PASS_BEGINE_OP_CLEAR ||
				op.ColorEnd < RENDER_PASS_END_OP_NONE || op.ColorEnd > RENDER_PASS_END_OP_STORE ||
				op.DepthStencilEnd < RENDER_PASS_END_OP_NONE || op.DepthStencilEnd > RENDER_PASS_END_OP_STORE)
				throw std::invalid_argument("Render-target load/store operation is invalid.");
			for (const auto& subpass : spec.Subpasses)
			{
				for (auto index : subpass.RenderTargetIndices)
					if (index >= spec.RenderTargets.size()) throw std::invalid_argument("Subpass color index is invalid.");
				for (auto index : subpass.InputRenderTargetIndices)
					if (index >= spec.RenderTargets.size()) throw std::invalid_argument("Subpass input index is invalid.");
				if (subpass.EnableDepthStencil && spec.DepthStencil == FRAME_BUFFER_TEXTURE_FORMAT_NONE)
					throw std::invalid_argument("Subpass requires a depth attachment.");
			}
			auto validate = [&](FrameBufferTextureFormat format, bool depth)
			{
				const auto support = QueryRenderTargetFormat(format);
				if (!(depth ? support.DepthStencilAttachment : support.ColorAttachment) ||
					!support.SupportsSamples(spec.Samples) || (!depth && !support.Sampled) ||
					(!depth && spec.Samples > 1 && !support.ColorResolve))
					throw std::invalid_argument("Render-target format/sample/resolve combination is unsupported.");
				if (depth && spec.Samples > 1 && !support.SupportsDepthResolve(spec.DepthResolve))
					throw std::invalid_argument("The requested multisample depth resolve mode is unsupported.");
				if (spec.Samples > 1 && !support.MultisampleLoad &&
					(depth ? spec.Operation.DepthStencilBegin : spec.Operation.ColorBegin) == RENDER_PASS_BEGINE_OP_LOAD)
					throw std::invalid_argument("Loading preserved multisample attachments is unsupported by this backend.");
				if (width > support.MaxWidth || height > support.MaxHeight)
					throw std::invalid_argument("Render-target dimensions exceed the format/framebuffer limit.");
			};
			for (const auto format : spec.RenderTargets) validate(format, false);
			if (spec.DepthStencil != FRAME_BUFFER_TEXTURE_FORMAT_NONE) validate(spec.DepthStencil, true);
		}
		virtual Ref<Texture2DCombineSampler> CreateTexture2DCombineSampler(const Ref<Texture2D>& texture, const Ref<Sampler>& sampler) = 0;
		virtual Ref<CubeMapCombineSampler> CreateCubeMapCombineSampler(const Ref<CubeMap>& cubemap, const Ref<Sampler>& sampler) = 0;
		virtual Ref<CubeMap> CreateCubeMap(uint32_t width, uint32_t height, bool generateMipmap, RenderImage2DFormat format) = 0;
		virtual Ref<CubeMap> CreateCubeMap(const std::string& rightPath, const std::string& leftPath, const std::string& topPath, const std::string& buttomPath, const std::string& backPath, const std::string& frontPath, bool generateMipmap) = 0;
		virtual Ref<Texture2DArray> CreateTexture2DArray(uint32_t width, uint32_t height, uint32_t layers, RenderImage2DFormat format) = 0;
		virtual void CopyTexture2DToArrayLayer(const Ref<CommandBuffer>& commandBuffer,
			const Ref<Texture2D>& source, const Ref<Texture2DArray>& destination, uint32_t layer) = 0;
		// Queue-family ownership applies only to backends with distinct native families.
		virtual void ReleaseTransferOwnership(const Ref<CommandBuffer>&, const Ref<GraphicsResource>&,
			const GraphicsResourceUsage&, const GraphicsSubresourceRange&) {}
		virtual void AcquireTransferOwnership(const Ref<CommandBuffer>&, const Ref<GraphicsResource>&,
			const GraphicsResourceUsage&, const GraphicsSubresourceRange&) {}
		virtual void ReleaseComputeOwnership(const Ref<CommandBuffer>&, const Ref<GraphicsResource>&,
			const GraphicsResourceUsage&, const GraphicsSubresourceRange&) {}
		virtual void AcquireComputeOwnership(const Ref<CommandBuffer>&, const Ref<GraphicsResource>&,
			const GraphicsResourceUsage&, const GraphicsSubresourceRange&) {}
		virtual QueueOwnershipKind GetQueueOwnershipKind(const Ref<GraphicsResource>&) const
		{ return QueueOwnershipKind::None; }
		virtual Ref<Shader> CreateShader(const std::string& path) = 0;
		virtual Ref<Material> CreateMaterial(const Ref<Shader>& shader, const std::string& name) = 0;
		virtual Ref<GraphicsPipeline> CreateGraphicsPipeline(const Ref<Material>& material, const Ref<VertexBuffer>& vertices) = 0;
		virtual Ref<ComputePipeline> CreateComputePipeline(const Ref<Material>& material) = 0;
		virtual Ref<Sampler> CreateSampler(const SamplerSpecification& specification) = 0;
		virtual Ref<StorageBuffer> CreateStorageBuffer(uint32_t size) = 0;
		virtual Ref<StorageImage2D> CreateStorageImage2D(uint32_t width, uint32_t height, ComputeImage2DFormat format) = 0;
		virtual void ValidateSubresourceRange(const Ref<GraphicsResource>& resource,
			const GraphicsSubresourceRange& range) const
		{
			const auto metadata = resource ? resource->GetSubresourceMetadata() : GraphicsSubresourceMetadata{};
			const bool wholeRange = metadata.MipLevelCount == 0 || metadata.ArrayLayerCount == 0
				? range.IsWholeResource()
				: range.IsWholeResource(metadata.MipLevelCount, metadata.ArrayLayerCount);
			if (!wholeRange) throw std::invalid_argument("This backend does not support the requested subresource range.");
		}

		virtual Ref<RenderPass> CreateRenderPass(const RenderPassSpecification& spec) = 0;
		virtual Ref<FrameBuffer> CreateFrameBuffer(const Ref<RenderPass>& renderPass, uint32_t width, uint32_t height) = 0;
		virtual Ref<FrameBuffer> ResizeFrameBuffer(const Ref<FrameBuffer>& buffer, uint32_t width, uint32_t height) = 0;

		virtual Ref<Texture2D> CreateTexture2D(uint32_t width, uint32_t height, RenderImage2DFormat format) = 0;
		virtual Ref<Texture2D> CreateTexture2D(uint32_t width, uint32_t height, RenderImage2DFormat format, uint32_t mipLevelCount) = 0;
		virtual Ref<Texture2D> CreateTexture2D(const std::string& path) = 0;
		virtual Ref<Texture2D> CreateTexture2D(uint32_t width, uint32_t height, void* data, uint32_t size, RenderImage2DFormat format) = 0;

		virtual Ref<UniformBuffer> CreateUniformBuffer(uint32_t size, uint32_t count, bool autoSetDataDynamic) = 0;
		virtual Ref<VertexBuffer> CreateVertexBuffer(uint32_t size, uint32_t sizeInstance, VertexTopology type) = 0;
		virtual Ref<VertexBuffer> CreateVertexBuffer(const void* vertices, uint32_t size, uint32_t sizeInstance, VertexTopology type) = 0;
		virtual Ref<IndexBuffer> CreateIndexBuffer(const uint32_t* indices, uint32_t count) = 0;
	};
}
