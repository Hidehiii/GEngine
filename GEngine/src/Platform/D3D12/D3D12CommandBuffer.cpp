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
        m_SingleTimeGraphicsCommandList.resize(Graphics::GetCommandBufferCount());
        for (int i = 0; i < Graphics::GetCommandBufferCount(); i++)
        {
            D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_GraphicsCommandList[i])));
            D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_ComputeCommandList[i])));
            D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_SingleTimeGraphicsCommandList[i])));
        }
    }
    D3D12CommandAllocator::~D3D12CommandAllocator()
    {
    }
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> D3D12CommandAllocator::BeginSingleTimeGraphicsCommand()
    {
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = m_SingleTimeGraphicsCommandList.at(m_SingleTimeGraphicsCommandListIndex++ % m_SingleTimeGraphicsCommandList.size());
        commandList->Reset(m_CommandAllocator.Get(), nullptr);
        return commandList;
    }
    void D3D12CommandAllocator::EndSingleTimeGraphicsCommand(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
    {
        D3D12_THROW_IF_FAILED(commandList->Close());
		ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
		D3D12Context::Get()->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists); 
    }
}