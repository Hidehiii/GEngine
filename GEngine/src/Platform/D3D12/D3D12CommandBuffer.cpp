#include "GEpch.h"
#include "D3D12CommandBuffer.h"
#include "D3D12Context.h"
#include "D3D12Utils.h"
#include "GEngine/Graphics/Graphics.h"

namespace GEngine
{
    D3D12CommandPool::D3D12CommandPool()
    {
		m_GraphicsList.resize(Graphics::GetCommandBufferCount());
		m_ComputeList.resize(Graphics::GetCommandBufferCount());
		m_TransferList.resize(Graphics::GetCommandBufferCount());

        for (int i = 0; i < Graphics::GetCommandBufferCount(); i++)
        {
            m_GraphicsList[i] = CreateRef<D3D12CommandBuffer>(CommandBufferType::Graphics);
            m_ComputeList[i] = CreateRef<D3D12CommandBuffer>(CommandBufferType::Compute);
            m_TransferList[i] = CreateRef<D3D12CommandBuffer>(CommandBufferType::Transfer);
        }
    }
    D3D12CommandPool::~D3D12CommandPool()
    {
    }
    Ref<D3D12CommandBuffer> D3D12CommandPool::GetCommandBuffer(CommandBufferType type)
    {
        if(type == CommandBufferType::Graphics)
            return m_GraphicsList.at(m_GraphicsListIndex++ % m_GraphicsList.size());
        if(type == CommandBufferType::Compute)
            return m_ComputeList.at(m_ComputeListIndex++ % m_ComputeList.size());
        if(type == CommandBufferType::Transfer)
            return m_TransferList.at(m_TransferListIndex++ % m_TransferList.size());
        return Ref<D3D12CommandBuffer>();
    }

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> D3D12CommandPool::BeginSingleTimeGraphicsCommand()
    {
        HANDLE fenceEvent = Utils::CreateFenceEvent(nullptr, FALSE, FALSE, nullptr);
        Utils::SetFenceValue(D3D12Context::Get()->GetGraphicsQueue(), m_SingleTimeCommandFence, 0, fenceEvent);
        CloseHandle(fenceEvent);

        m_SingleTimeGraphicsCommandList->Reset(m_CommandAllocator.Get(), nullptr);
        return m_SingleTimeGraphicsCommandList;
    }
    void D3D12CommandPool::EndSingleTimeGraphicsCommand(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
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
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> D3D12CommandPool::BeginSingleTimeComputeCommand()
    {
        return Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>();
    }
    void D3D12CommandPool::EndSingleTimeComputeCommand(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
    {
    }
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> D3D12CommandPool::BeginSingleTimeTransferCommand()
    {
        return Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>();
    }
    void D3D12CommandPool::EndSingleTimeTransferCommand(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
    {
    }
    D3D12CommandBuffer::D3D12CommandBuffer(CommandBufferType type)
    {
        m_Type          = type;

        if (m_Type == CommandBufferType::Graphics)
        {
            D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_Allocator)));
            D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_Allocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList)));
        }
        if (m_Type == CommandBufferType::Compute)
        {
			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&m_Allocator)));
			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, m_Allocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList)));
        }
        if (m_Type == CommandBufferType::Transfer)
        {
			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&m_Allocator)));
			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_Allocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList)));
        }
    }
    D3D12CommandBuffer::~D3D12CommandBuffer()
    {
        m_Allocator->Reset();
        m_Allocator->Release();
    }
    void D3D12CommandBuffer::Begin(Ref<FrameBuffer>& buffer)
    {
        D3D12_THROW_IF_FAILED(m_CommandList->Reset(m_Allocator.Get(), nullptr));


		if (m_Type == CommandBufferType::Graphics)
		{
			GE_CORE_ASSERT(buffer != nullptr, "graphics cmd must have frame buffer");
			m_FrameBuffer = std::static_pointer_cast<D3D12CommandBuffer>(buffer);
			m_FrameBuffer->Begin(this);
		}

		if (buffer != nullptr)
			Graphics::UpdateScreenUniform(Vector4{ buffer->GetWidth(), buffer->GetHeight(), 0, 0 });
    }
    void D3D12CommandBuffer::End()
    {
		if (m_Type == CommandBufferType::Graphics)
		{
			m_FrameBuffer->End(this);
		}

        D3D12_THROW_IF_FAILED(m_CommandList->Close());
        // TODO: 如何多list 之间同步？最好不用在cpu wait
    }
}