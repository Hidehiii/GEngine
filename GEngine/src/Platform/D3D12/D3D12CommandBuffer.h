#pragma once
#include "GEngine/Graphics/CommandBuffer.h"
#include "D3D12FrameBuffer.h"
#include <windows.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d12.h>
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

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	GetCommandList() { return m_CommandList; }
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator>		GetCommandAllocator() { return m_Allocator; }
	private:
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	m_CommandList;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator>		m_Allocator;
		Ref<D3D12FrameBuffer>								m_FrameBuffer;
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

