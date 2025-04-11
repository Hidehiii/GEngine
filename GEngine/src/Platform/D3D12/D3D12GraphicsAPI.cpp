#include "GEpch.h"
#include "D3D12GraphicsAPI.h"
#include "D3D12Context.h"


namespace GEngine
{
    D3D12GraphicsAPI::D3D12GraphicsAPI()
    {
        s_API = GraphicsAPI::API::Direct3DX12;
    }
    D3D12GraphicsAPI::~D3D12GraphicsAPI()
    {
    }
    Ref<CommandBuffer> D3D12GraphicsAPI::GetGraphicsCommandBuffer()
    {
        return D3D12Context::Get()->GetCommandBuffer(CommandBufferType::Graphics);
    }
    Ref<CommandBuffer> D3D12GraphicsAPI::GetComputeCommandBuffer()
    {
        return D3D12Context::Get()->GetCommandBuffer(CommandBufferType::Compute);
    }
    std::vector<std::string> D3D12GraphicsAPI::GetExtensions()
    {
        return std::vector<std::string>();
    }
    uint32_t D3D12GraphicsAPI::GetMaxTextureSize()
    {
        return uint32_t();
    }
    uint32_t D3D12GraphicsAPI::GetMaxCombinedTextureCount()
    {
        return uint32_t();
    }
    uint32_t D3D12GraphicsAPI::GetMaxPerStageTextureCount()
    {
        return uint32_t();
    }
    uint32_t D3D12GraphicsAPI::GetMaxTextureArrayLayers()
    {
        return uint32_t();
    }
    uint32_t D3D12GraphicsAPI::GetMinUniformBufferOffsetAlignment()
    {
        return uint32_t();
    }
    uint32_t D3D12GraphicsAPI::GetMaxUniformBufferSize()
    {
        return uint32_t();
    }
    Vector3 D3D12GraphicsAPI::GetMaxComputeWorkGroupCount()
    {
        return Vector3();
    }
    Vector3 D3D12GraphicsAPI::GetMaxComputeWorkGroupSize()
    {
        return Vector3();
    }
    uint32_t D3D12GraphicsAPI::GetMaxComputeWorkGroupInvocations()
    {
        return uint32_t();
    }
    void D3D12GraphicsAPI::SetCommandsBarrier(Ref<CommandBuffer>& first, Ref<CommandBuffer>& second)
    {
        auto f = D3D12Context::Get()->GetFence();
		std::dynamic_pointer_cast<D3D12CommandBuffer>(first)->AddSignalFence(f);
		std::dynamic_pointer_cast<D3D12CommandBuffer>(second)->AddWaitFence(f);
    }
}