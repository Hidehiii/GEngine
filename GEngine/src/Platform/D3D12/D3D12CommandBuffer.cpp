#include "GEpch.h"
#include "D3D12CommandBuffer.h"
#include "D3D12Context.h"
#include "D3D12Utils.h"
#include "GEngine/Graphics/Graphics.h"

namespace GEngine
{
    D3D12CommandAllocator::D3D12CommandAllocator()
    {
        D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator)));

        m_GraphicsCommandList.resize(Graphics::GetCommandBufferCount());
        m_ComputeCommandList.resize(Graphics::GetCommandBufferCount());
        for (int i = 0; i < Graphics::GetCommandBufferCount(); i++)
        {
            D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_GraphicsCommandList[i])));
            D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_ComputeCommandList[i])));
            D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_TransferCommandList[i])));
        }

		D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_SingleTimeGraphicsCommandList)));
		D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_SingleTimeComputeCommandList)));
		D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_SingleTimeTransferCommandList)));

        D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_SingleTimeCommandFence)));
    }
    D3D12CommandAllocator::~D3D12CommandAllocator()
    {
        m_CommandAllocator->Reset();
        m_CommandAllocator->Release();
    }
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> D3D12CommandAllocator::BeginSingleTimeGraphicsCommand()
    {
        HANDLE fenceEvent = Utils::CreateFenceEvent(nullptr, FALSE, FALSE, nullptr);
        Utils::SetFenceValue(D3D12Context::Get()->GetGraphicsQueue(), m_SingleTimeCommandFence, 0, fenceEvent);
        CloseHandle(fenceEvent);

        m_SingleTimeGraphicsCommandList->Reset(m_CommandAllocator.Get(), nullptr);
        return m_SingleTimeGraphicsCommandList;
    }
    void D3D12CommandAllocator::EndSingleTimeGraphicsCommand(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
    {
        D3D12_THROW_IF_FAILED(commandList->Close());
		ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
		D3D12Context::Get()->GetGraphicsQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists); 

        const UINT64 fenceValue = 1;
        D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetGraphicsQueue()->Signal(m_SingleTimeCommandFence.Get(), fenceValue));
        HANDLE fenceEvent = Utils::CreateFenceEvent(nullptr, FALSE, FALSE, nullptr);
        Utils::WaitForFence(D3D12Context::Get()->GetGraphicsQueue(), m_SingleTimeCommandFence, fenceValue, fenceEvent);
        CloseHandle(fenceEvent);
    }
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> D3D12CommandAllocator::BeginSingleTimeComputeCommand()
    {
        return Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>();
    }
    void D3D12CommandAllocator::EndSingleTimeComputeCommand(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
    {
    }
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> D3D12CommandAllocator::BeginSingleTimeTransferCommand()
    {
        return Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>();
    }
    void D3D12CommandAllocator::EndSingleTimeTransferCommand(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
    {
    }
}