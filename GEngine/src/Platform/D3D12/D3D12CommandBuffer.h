#pragma once
#include "GEngine/Graphics/CommandBuffer.h"
#include "D3D12FrameBuffer.h"
#include <directx/d3dx12.h>
namespace GEngine
{
	class GENGINE_API D3D12CommandBuffer : public CommandBuffer
	{
	public:
		D3D12CommandBuffer(CommandBufferType type);

		virtual ~D3D12CommandBuffer();

		virtual void Begin(Ref<FrameBuffer>& buffer) override;

		virtual void End() override;

		virtual void Render(Ref<GraphicsPipeline>& pipeline, int pass, uint32_t instanceCount = 1, uint32_t indexCount = 0) override;

		virtual void Compute(Ref<ComputePipeline>& pipeline, int pass, uint32_t x, uint32_t y, uint32_t z) override;

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	GetCommandList() { return m_CommandList; }
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator>		GetCommandAllocator() { return m_Allocator; }

		void AddSignalFence(std::pair<Microsoft::WRL::ComPtr<ID3D12Fence>, uint64_t> f) { m_SignalFences.push_back(f); }
		void AddWaitFence(std::pair<Microsoft::WRL::ComPtr<ID3D12Fence>, uint64_t> f) { m_WaitFences.push_back(f); }

		void ClearSignalFence() { m_SignalFences.clear(); }
		void ClearWaitFence() { m_WaitFences.clear(); }
	private:
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>						m_CommandList;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator>							m_Allocator;
		Ref<D3D12FrameBuffer>													m_FrameBuffer;
		std::vector<std::pair<Microsoft::WRL::ComPtr<ID3D12Fence>, uint64_t>>	m_SignalFences;
		std::vector<std::pair<Microsoft::WRL::ComPtr<ID3D12Fence>, uint64_t>>	m_WaitFences;
	};

	class GENGINE_API D3D12CommandPool
	{
	public:
		D3D12CommandPool();
		~D3D12CommandPool();

		Ref<D3D12CommandBuffer>	GetCommandBuffer(CommandBufferType type);

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	BeginSingleTimeGraphicsCommand();
		void												EndSingleTimeGraphicsCommand(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	BeginSingleTimeComputeCommand();
		void												EndSingleTimeComputeCommand(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	BeginSingleTimeTransferCommand();
		void												EndSingleTimeTransferCommand(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);

	private:
		std::vector<Ref<D3D12CommandBuffer>>		m_GraphicsList;
		std::vector<Ref<D3D12CommandBuffer>>		m_ComputeList;
		std::vector<Ref<D3D12CommandBuffer>>		m_TransferList;
		uint16_t									m_GraphicsListIndex = 0;
		uint16_t									m_ComputeListIndex = 0;
		uint16_t									m_TransferListIndex = 0;

	};

	

}

