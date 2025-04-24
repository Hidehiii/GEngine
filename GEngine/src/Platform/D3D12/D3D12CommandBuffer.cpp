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
            m_GraphicsList[i]   = CreateRef<D3D12CommandBuffer>(CommandBufferType::Graphics);
            m_ComputeList[i]    = CreateRef<D3D12CommandBuffer>(CommandBufferType::Compute);
            m_TransferList[i]   = CreateRef<D3D12CommandBuffer>(CommandBufferType::Transfer);
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
        return Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>();
	}
	void D3D12CommandPool::EndSingleTimeGraphicsCommand(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
	{
		
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
			m_FrameBuffer = std::static_pointer_cast<D3D12FrameBuffer>(buffer);
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

        Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue;
        if (m_Type == CommandBufferType::Graphics)
            queue = D3D12Context::Get()->GetGraphicsQueue();
        if (m_Type == CommandBufferType::Compute)
            queue = D3D12Context::Get()->GetComputeQueue();
        if (m_Type == CommandBufferType::Transfer)
            queue = D3D12Context::Get()->GetTransferQueue();

        for (auto wait : m_WaitFences)
        {
           queue->Wait(wait.first.Get(), wait.second);
        }
           queue->ExecuteCommandLists(1, CommandListCast(m_CommandList.GetAddressOf()));

        for (auto signal : m_SignalFences)
        {
            queue->Signal(signal.first.Get(), signal.second);
        }

        ClearWaitFence();
        ClearSignalFence();
    }
    void D3D12CommandBuffer::Render(Ref<GraphicsPipeline>& pipeline, std::string pass, uint32_t instanceCount, uint32_t indexCount)
    {
    }
    void D3D12CommandBuffer::Compute(Ref<ComputePipeline>& pipeline, std::string pass, uint32_t x, uint32_t y, uint32_t z)
    {
    }
}