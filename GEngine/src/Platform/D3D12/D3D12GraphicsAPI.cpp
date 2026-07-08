#include "GEpch.h"
#include "D3D12GraphicsAPI.h"
#include "D3D12Context.h"


namespace GEngine
{
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
        D3D12Context::Get()->IncreaseFenceValue(first->GetType());
        auto f = D3D12Context::Get()->GetFence(first->GetType());
		std::dynamic_pointer_cast<D3D12CommandBuffer>(first)->AddSignalFence(f);
		std::dynamic_pointer_cast<D3D12CommandBuffer>(second)->AddWaitFence(f);
    }
}