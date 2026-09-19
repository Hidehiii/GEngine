#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsCommon.h"
#include "GEngine/Graphics/GraphicsQueue.h"
#include "GEngine/Graphics/ImageFormat.h"

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

	class GENGINE_API RenderDevice
	{
	public:
		virtual ~RenderDevice() = default;
		virtual GraphicsQueue& GetQueue(CommandBufferType type) = 0;
		virtual Ref<Texture2DCombineSampler> CreateTexture2DCombineSampler(const Ref<Texture2D>& texture, const Ref<Sampler>& sampler) = 0;
		virtual Ref<CubeMapCombineSampler> CreateCubeMapCombineSampler(const Ref<CubeMap>& cubemap, const Ref<Sampler>& sampler) = 0;
		virtual Ref<CubeMap> CreateCubeMap(uint32_t width, uint32_t height, bool generateMipmap, RenderImage2DFormat format) = 0;
		virtual Ref<CubeMap> CreateCubeMap(const std::string& rightPath, const std::string& leftPath, const std::string& topPath, const std::string& buttomPath, const std::string& backPath, const std::string& frontPath, bool generateMipmap) = 0;
		virtual Ref<Texture2DArray> CreateTexture2DArray(uint32_t width, uint32_t height, uint32_t layers, RenderImage2DFormat format) = 0;
		virtual Ref<Shader> CreateShader(const std::string& path) = 0;
		virtual Ref<Material> CreateMaterial(const Ref<Shader>& shader, const std::string& name) = 0;
		virtual Ref<GraphicsPipeline> CreateGraphicsPipeline(const Ref<Material>& material, const Ref<VertexBuffer>& vertices) = 0;
		virtual Ref<ComputePipeline> CreateComputePipeline(const Ref<Material>& material) = 0;
		virtual Ref<Sampler> CreateSampler(const SamplerSpecification& specification) = 0;
		virtual Ref<StorageBuffer> CreateStorageBuffer(uint32_t size) = 0;
		virtual Ref<StorageImage2D> CreateStorageImage2D(uint32_t width, uint32_t height, ComputeImage2DFormat format) = 0;

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
