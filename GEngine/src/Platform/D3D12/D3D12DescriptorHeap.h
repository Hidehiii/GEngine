#pragma once
#include "GEngine/Core/Core.h"
#include <windows.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <directx/d3dx12.h>
#include <set>

namespace GEngine
{
	class GENGINE_API D3D12DescriptorHeap
	{
	public:
		struct D3D12DescriptorAllocationInfo
		{
			std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE>	CpuHandles;
			std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE>	GpuHandles;
			uint32_t									StartIndex = UINT32_MAX;
			uint32_t									Count = 0;
			D3D12_DESCRIPTOR_HEAP_TYPE					Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

			bool IsValid() const { return StartIndex != UINT32_MAX && Count > 0; }
		};

		D3D12DescriptorHeap(uint32_t rtvCount = 10000, uint32_t dsvCount = 10000, uint32_t cbvSrvUavCount = 10000);
		~D3D12DescriptorHeap();

		D3D12DescriptorAllocationInfo	Allocate(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t count);
		void							Free(const D3D12DescriptorAllocationInfo& info);

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetRtvDescriptorHeap(const uint32_t& frame) const { return m_RtvHeapInfo.Heaps.at(frame); }
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDsvDescriptorHeap(const uint32_t& frame) const { return m_DsvHeapInfo.Heaps.at(frame); }
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetCbvSrvUavDescriptorHeap(const uint32_t& frame) const { return m_CbvSrvUavHeapInfo.Heaps.at(frame); }
	private:
		struct DescriptorHeapInfo
		{
			std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>>	Heaps; // each frame has its own heap
			uint32_t													DescriptorCount = 0;
			std::vector<uint8_t>										DescriptorUsage; // 0 = free, 1 = used  
			std::set<uint32_t>											FreeIndices; // free descriptor indices
		};

		D3D12DescriptorAllocationInfo	AllocateFromHeap(DescriptorHeapInfo& heapInfo, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t count);
		void							FreeToHeap(DescriptorHeapInfo& heapInfo, const D3D12DescriptorAllocationInfo& info);
	private:
		
		DescriptorHeapInfo m_RtvHeapInfo;
		DescriptorHeapInfo m_DsvHeapInfo;
		DescriptorHeapInfo m_CbvSrvUavHeapInfo;
	};
}


