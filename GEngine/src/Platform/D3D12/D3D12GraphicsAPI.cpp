#include "GEpch.h"
#include "Platform/D3D12/D3D12TextureCombineSampler.h"
#include "Platform/D3D12/D3D12CubeMap.h"
#include "Platform/D3D12/D3D12Texture2DArray.h"
#include "Platform/D3D12/D3D12Shader.h"
#include "Platform/D3D12/D3D12Material.h"
#include "Platform/D3D12/D3D12GraphicsPipeline.h"
#include "Platform/D3D12/D3D12ComputePipeline.h"
#include "Platform/D3D12/D3D12Sampler.h"
#include "Platform/D3D12/D3D12StorageBuffer.h"
#include "Platform/D3D12/D3D12StorageImage2D.h"
#include "Platform/D3D12/D3D12GraphicsAPI.h"
#include "Platform/D3D12/D3D12Utils.h"
#include "Platform/D3D12/D3D12Context.h"
#include "Platform/D3D12/D3D12RenderPass.h"
#include "Platform/D3D12/D3D12FrameBuffer.h"
#include "Platform/D3D12/D3D12Texture2D.h"
#include "Platform/D3D12/D3D12UniformBuffer.h"
#include "Platform/D3D12/D3D12VertexBuffer.h"

namespace
{
	D3D12_RESOURCE_STATES ToD3D12ResourceState(GEngine::GraphicsResourceState state)
	{
		using State = GEngine::GraphicsResourceState;
		switch (state)
		{
		case State::RenderTarget: return D3D12_RESOURCE_STATE_RENDER_TARGET;
		case State::DepthWrite: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
		case State::ShaderRead: return D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
		case State::ShaderWrite: return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		case State::CopySource: return D3D12_RESOURCE_STATE_COPY_SOURCE;
		case State::CopyDestination: return D3D12_RESOURCE_STATE_COPY_DEST;
		case State::Present: return D3D12_RESOURCE_STATE_PRESENT;
		case State::Undefined: return D3D12_RESOURCE_STATE_COMMON;
		}
		return D3D12_RESOURCE_STATE_COMMON;
	}
}

namespace GEngine
{
	Ref<Texture2DCombineSampler> D3D12GraphicsAPI::CreateTexture2DCombineSampler(const Ref<Texture2D>& texture, const Ref<Sampler>& sampler)
	{
		return CreateRef<D3D12Texture2DCombineSampler>(texture, sampler);
	}
	Ref<CubeMapCombineSampler> D3D12GraphicsAPI::CreateCubeMapCombineSampler(const Ref<CubeMap>& cubemap, const Ref<Sampler>& sampler)
	{
		return CreateRef<D3D12CubeMapCombineSampler>(cubemap, sampler);
	}
	Ref<CubeMap> D3D12GraphicsAPI::CreateCubeMap(uint32_t width, uint32_t height, bool generateMipmap, RenderImage2DFormat format)
	{
		return CreateRef<D3D12CubeMap>(width, height, generateMipmap, format);
	}
	Ref<CubeMap> D3D12GraphicsAPI::CreateCubeMap(const std::string& rightPath, const std::string& leftPath, const std::string& topPath, const std::string& buttomPath, const std::string& backPath, const std::string& frontPath, bool generateMipmap)
	{
		return CreateRef<D3D12CubeMap>(rightPath, leftPath, topPath, buttomPath, backPath, frontPath, generateMipmap);
	}
	Ref<Texture2DArray> D3D12GraphicsAPI::CreateTexture2DArray(uint32_t width, uint32_t height, uint32_t layers, RenderImage2DFormat format)
	{
		return CreateRef<D3D12Texture2DArray>(width, height, layers, format);
	}
	Ref<Shader> D3D12GraphicsAPI::CreateShader(const std::string& path)
	{
		return CreateRef<D3D12Shader>(path);
	}
	Ref<Material> D3D12GraphicsAPI::CreateMaterial(const Ref<Shader>& shader, const std::string& name)
	{
		return CreateRef<D3D12Material>(shader, name);
	}
	Ref<GraphicsPipeline> D3D12GraphicsAPI::CreateGraphicsPipeline(const Ref<Material>& material, const Ref<VertexBuffer>& vertices)
	{
		return CreateRef<D3D12GraphicsPipeline>(material, vertices);
	}
	Ref<ComputePipeline> D3D12GraphicsAPI::CreateComputePipeline(const Ref<Material>& material)
	{
		return CreateRef<D3D12ComputePipeline>(material);
	}
	Ref<Sampler> D3D12GraphicsAPI::CreateSampler(const SamplerSpecification& specification)
	{
		return CreateRef<D3D12Sampler>(specification);
	}
	Ref<StorageBuffer> D3D12GraphicsAPI::CreateStorageBuffer(uint32_t size)
	{
		return CreateRef<D3D12StorageBuffer>(size);
	}
	Ref<StorageImage2D> D3D12GraphicsAPI::CreateStorageImage2D(uint32_t width, uint32_t height, ComputeImage2DFormat format)
	{
		return CreateRef<D3D12StorageImage2D>(width, height, format);
	}
    D3D12GraphicsAPI::D3D12GraphicsAPI()
    {
        s_API = GRAPHICS_API_DIRECT3DX12;
    }
    D3D12GraphicsAPI::~D3D12GraphicsAPI()
    {
    }
    Ref<CommandBuffer> D3D12GraphicsAPI::GetGraphicsCommandBuffer()
    {
        return D3D12Context::Get()->GetCommandBuffer(COMMAND_BUFFER_TYPE_GRAPHICS);
    }
    Ref<CommandBuffer> D3D12GraphicsAPI::GetComputeCommandBuffer()
    {
        return D3D12Context::Get()->GetCommandBuffer(COMMAND_BUFFER_TYPE_COMPUTE);
    }
    std::vector<std::string> D3D12GraphicsAPI::GetExtensions()
    {
		// incomplete, need to check for features and extensions
        std::vector<std::string> exts;
        D3D12_FEATURE_DATA_D3D12_OPTIONS opts{};
		D3D12Context::Get()->GetDevice()->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &opts, sizeof(opts));

        // base features
        exts.emplace_back("D3D12_BASE");
        exts.emplace_back("TEXTURE_2D_ARRAY");
        exts.emplace_back("TEXTURE_3D");
        exts.emplace_back("COMPUTE_SHADER");
        exts.emplace_back("ROOT_SIGNATURE");
        exts.emplace_back("CONSTANT_BUFFER_DYNAMIC_OFFSET");

        if(opts.TypedUAVLoadAdditionalFormats)
			exts.emplace_back("TYPED_UAV_LOAD_ADDITIONAL_FORMATS");
        return exts;
    }
    GraphicsCapabilities D3D12GraphicsAPI::GetCapabilities() const
    {
        GraphicsCapabilities capabilities;
        capabilities.RenderPass = true;
        capabilities.FrameBuffer = true;
        capabilities.Texture2D = true;
        capabilities.Texture2DArray = true;
        capabilities.CubeMap = true;
        capabilities.Sampler = true;
        capabilities.UniformBuffer = true;
        capabilities.StorageBuffer = true;
        capabilities.StorageImage = true;
        capabilities.Compute = true;
        return capabilities;
    }
    Ref<RenderPass> D3D12GraphicsAPI::CreateRenderPass(const RenderPassSpecification& spec)
    {
        return CreateRef<D3D12RenderPass>(spec);
    }
    Ref<FrameBuffer> D3D12GraphicsAPI::CreateFrameBuffer(const Ref<RenderPass>& renderPass, uint32_t width, uint32_t height)
    {
        return CreateRef<D3D12FrameBuffer>(renderPass, width, height);
    }
    Ref<FrameBuffer> D3D12GraphicsAPI::ResizeFrameBuffer(const Ref<FrameBuffer>& buffer, uint32_t width, uint32_t height)
    {
        return CreateRef<D3D12FrameBuffer>(buffer, width, height);
    }
    Ref<Texture2D> D3D12GraphicsAPI::CreateTexture2D(uint32_t width, uint32_t height, RenderImage2DFormat format)
    {
        return CreateRef<D3D12Texture2D>(width, height, format);
    }
    Ref<Texture2D> D3D12GraphicsAPI::CreateTexture2D(const std::string& path)
    {
        return CreateRef<D3D12Texture2D>(path);
    }
    Ref<Texture2D> D3D12GraphicsAPI::CreateTexture2D(uint32_t width, uint32_t height, void* data, uint32_t size, RenderImage2DFormat format)
    {
        return CreateRef<D3D12Texture2D>(width, height, data, size, format);
    }
    Ref<UniformBuffer> D3D12GraphicsAPI::CreateUniformBuffer(uint32_t size, uint32_t count, bool autoSetDataDynamic)
    {
        return CreateRef<D3D12UniformBuffer>(size, count, autoSetDataDynamic);
    }
    Ref<VertexBuffer> D3D12GraphicsAPI::CreateVertexBuffer(uint32_t size, uint32_t sizeInstance, VertexTopology type)
    {
        return CreateRef<D3D12VertexBuffer>(size, sizeInstance, type);
    }
    Ref<VertexBuffer> D3D12GraphicsAPI::CreateVertexBuffer(const void* vertices, uint32_t size, uint32_t sizeInstance, VertexTopology type)
    {
        return CreateRef<D3D12VertexBuffer>(vertices, size, sizeInstance, type);
    }
    Ref<IndexBuffer> D3D12GraphicsAPI::CreateIndexBuffer(const uint32_t* indices, uint32_t count)
    {
        return CreateRef<D3D12IndexBuffer>(indices, count);
    }
    uint32_t D3D12GraphicsAPI::GetMaxTexture2DSize()
    {
        // Direct3D12 doesn't expose a single query like Vulkan; use the common required constant.
        // D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION is the required maximum 2D texture dimension.
        // Use a conservative literal fallback if the constant is unavailable.
#ifdef D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION
		return static_cast<uint32_t>(D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION);
#else
        return 16384u;
#endif
    }
    uint32_t D3D12GraphicsAPI::GetMaxCombinedTextureCount()
    {
        // Use common shader register count as a conservative estimate.
#ifdef D3D12_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT
		return static_cast<uint32_t>(D3D12_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
#else
        return 128u;
#endif
    }
    uint32_t D3D12GraphicsAPI::GetMaxPerStageTextureCount()
    {
#ifdef D3D12_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT
		return static_cast<uint32_t>(D3D12_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
#else
		return 128u;
#endif
    }
    uint32_t D3D12GraphicsAPI::GetMaxTextureArrayLayers()
    {
#ifdef D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION
		return static_cast<uint32_t>(D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION);
#else
		return 2048u;
#endif
    }
    uint32_t D3D12GraphicsAPI::GetMinUniformBufferOffsetAlignment()
    {
        // CBV alignment is 256 bytes on D3D12.
#ifdef D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT
		return static_cast<uint32_t>(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
#else
		return 256u;
#endif
    }
    uint32_t D3D12GraphicsAPI::GetMaxUniformBufferSize()
    {
        // Use the constant-buffer element count if available (elements are 16 bytes each).
#ifdef D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT
        return static_cast<uint32_t>(D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT);
#else
		return 65536u; // 64KB
#endif
    }
    Vector3 D3D12GraphicsAPI::GetMaxComputeWorkGroupCount()
    {
        // Conservative common values: large dispatch counts per dimension.
        return Vector3(65535, 65535, 65535);
    }
    Vector3 D3D12GraphicsAPI::GetMaxComputeWorkGroupSize()
    {
        // Conservative per-dimension thread group size limits.
        // Many HLSL implementations allow up to 1024 threads per group (e.g., 1024x1x1),
        // but to be conservative across hardware choose typical limits.
        return Vector3(1024, 1024, 64);
    }
    uint32_t D3D12GraphicsAPI::GetMaxComputeWorkGroupInvocations()
    {
        // Conservative typical maximum threads per group.
        return 1024u;
    }
    void D3D12GraphicsAPI::SetCommandsBarrier(Ref<CommandBuffer>& first, Ref<CommandBuffer>& second)
    {
		GE_CORE_ASSERT(first && second, "Command barriers require two command buffers.");
		// Submissions to the same D3D12 queue are already ordered.  Adding a queue
		// wait here can deadlock if callers submit the two buffers in reverse order.
		if (first->GetType() == second->GetType())
			return;

		auto firstD3D = std::dynamic_pointer_cast<D3D12CommandBuffer>(first);
		auto secondD3D = std::dynamic_pointer_cast<D3D12CommandBuffer>(second);
		GE_CORE_ASSERT(firstD3D && secondD3D, "D3D12 barriers require D3D12 command buffers.");
		secondD3D->AddWaitFence(firstD3D->NextCompletion());
    }

	void D3D12GraphicsAPI::SubmitCommandBuffer(const Ref<CommandBuffer>& commandBuffer)
	{
		auto d3dCommandBuffer = std::dynamic_pointer_cast<D3D12CommandBuffer>(commandBuffer);
		GE_CORE_ASSERT(d3dCommandBuffer, "D3D12 submission requires a D3D12 command buffer.");

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue;
		switch (d3dCommandBuffer->GetType())
		{
		case COMMAND_BUFFER_TYPE_GRAPHICS: queue = D3D12Context::Get()->GetGraphicsQueue(); break;
		case COMMAND_BUFFER_TYPE_COMPUTE: queue = D3D12Context::Get()->GetComputeQueue(); break;
		case COMMAND_BUFFER_TYPE_TRANSFER: queue = D3D12Context::Get()->GetTransferQueue(); break;
		default: GE_CORE_ASSERT(false, "D3D12 command buffer type is invalid."); return;
		}

		for (const auto& wait : d3dCommandBuffer->GetWaitFences())
			D3D12_THROW_IF_FAILED(queue->Wait(wait.first.Get(), wait.second));
		auto commandList = d3dCommandBuffer->GetCommandList();
		queue->ExecuteCommandLists(1, CommandListCast(commandList.GetAddressOf()));
		d3dCommandBuffer->MarkSubmitted(queue.Get());
		for (const auto& signal : d3dCommandBuffer->GetSignalFences())
			D3D12_THROW_IF_FAILED(queue->Signal(signal.first.Get(), signal.second));
		d3dCommandBuffer->ClearWaitFences();
		d3dCommandBuffer->ClearSignalFences();
	}

	void D3D12GraphicsAPI::WaitForIdle()
	{
		D3D12Context::Get()->WaitForFence(COMMAND_BUFFER_TYPE_GRAPHICS);
		D3D12Context::Get()->WaitForFence(COMMAND_BUFFER_TYPE_COMPUTE);
		D3D12Context::Get()->WaitForFence(COMMAND_BUFFER_TYPE_TRANSFER);
	}

	void D3D12GraphicsAPI::TransitionResource(const Ref<CommandBuffer>& commandBuffer, const Ref<GraphicsResource>& resource,
		GraphicsResourceState before, GraphicsResourceState after)
	{
		const auto nativeResource = GetNativeResource(resource);
		if (!resource || nativeResource == nullptr)
			return;

		auto d3dCommandBuffer = std::dynamic_pointer_cast<D3D12CommandBuffer>(commandBuffer);
		GE_CORE_ASSERT(d3dCommandBuffer, "D3D12 resource transitions require a D3D12 command buffer.");
		d3dCommandBuffer->Retain(resource);
		if (before == after)
		{
			if (before == GraphicsResourceState::ShaderWrite)
			{
				auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(static_cast<ID3D12Resource*>(nativeResource));
				d3dCommandBuffer->GetCommandList()->ResourceBarrier(1, &barrier);
			}
			return;
		}
		if (auto texture = std::dynamic_pointer_cast<D3D12Texture2D>(resource))
		{
			texture->TransitionResourceState(d3dCommandBuffer->GetCommandList(), ToD3D12ResourceState(after));
			return;
		}
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(static_cast<ID3D12Resource*>(nativeResource),
			ToD3D12ResourceState(before), ToD3D12ResourceState(after));
		d3dCommandBuffer->GetCommandList()->ResourceBarrier(1, &barrier);
	}
}
