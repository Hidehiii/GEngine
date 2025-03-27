#pragma once
#include "GEngine/Graphics/CommandBuffer.h"
#include <windows.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <directx/d3dx12.h>
namespace GEngine
{
	class GENGINE_API D3D12CommandAllocator
	{
	public:
		D3D12CommandAllocator();
		~D3D12CommandAllocator();

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	BeginSingleTimeGraphicsCommand();
		void												EndSingleTimeGraphicsCommand(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator>	GetCommandAllocator() { return m_CommandAllocator; }
	private:
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator>					m_CommandAllocator;
		std::vector<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>>	m_GraphicsCommandList;
		std::vector<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>>	m_ComputeCommandList;
		std::vector<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>>	m_SingleTimeGraphicsCommandList;
		uint32_t														m_SingleTimeGraphicsCommandListIndex = 0;
	};

	class GENGINE_API D3D12CommandBuffer : public CommandBuffer
	{
	};

}

