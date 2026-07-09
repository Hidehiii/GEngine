#include "GEpch.h"
#include "D3D12DescriptorHeap.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/D3D12/D3D12Utils.h"
#include "Platform/D3D12/D3D12Context.h"

namespace GEngine
{
	D3D12DescriptorHeap::D3D12DescriptorHeap(uint32_t rtvCount, uint32_t dsvCount, uint32_t cbvSrvUavCount)
	{
		// rtv
		m_RtvHeapInfo.DescriptorCount = rtvCount;
		m_RtvHeapInfo.DescriptorUsage = std::vector<uint8_t>(rtvCount, 0);
		m_RtvHeapInfo.Heaps.resize(Graphics::GetFrameCount());
		m_RtvHeapInfo.FreeIndices.insert(0);

		D3D12_DESCRIPTOR_HEAP_DESC	heapDesc = {};
		heapDesc.NumDescriptors		= rtvCount;
		heapDesc.Type				= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heapDesc.Flags				= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		for (int i = 0; i < Graphics::GetFrameCount(); i++)
		{
			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_RtvHeapInfo.Heaps[i])));
		}
		// dsv
		m_DsvHeapInfo.DescriptorCount = dsvCount;
		m_DsvHeapInfo.DescriptorUsage = std::vector<uint8_t>(dsvCount, 0);
		m_DsvHeapInfo.Heaps.resize(Graphics::GetFrameCount());
		m_DsvHeapInfo.FreeIndices.insert(0);

		heapDesc.NumDescriptors = dsvCount;
		heapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		heapDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		for(int i = 0; i < Graphics::GetFrameCount(); i++)
		{
			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DsvHeapInfo.Heaps[i])));
		}
		// cbv srv uav
		m_CbvSrvUavHeapInfo.DescriptorCount = cbvSrvUavCount;
		m_CbvSrvUavHeapInfo.DescriptorUsage = std::vector<uint8_t>(cbvSrvUavCount, 0);
		m_CbvSrvUavHeapInfo.Heaps.resize(Graphics::GetFrameCount());
		m_CbvSrvUavHeapInfo.FreeIndices.insert(0);

		heapDesc.NumDescriptors = cbvSrvUavCount;
		heapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heapDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		for(int i = 0; i < Graphics::GetFrameCount(); i++)
		{
			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_CbvSrvUavHeapInfo.Heaps[i])));
		}
	}

	D3D12DescriptorHeap::~D3D12DescriptorHeap()
	{
		D3D12Context::Get()->WaitForFence(COMMAND_BUFFER_TYPE_GRAPHICS);
		D3D12Context::Get()->WaitForFence(COMMAND_BUFFER_TYPE_COMPUTE);
		D3D12Context::Get()->WaitForFence(COMMAND_BUFFER_TYPE_TRANSFER);

		for(int i = 0; i < Graphics::GetFrameCount(); i++)
		{
			m_RtvHeapInfo.Heaps[i].Reset();
			m_DsvHeapInfo.Heaps[i].Reset();
			m_CbvSrvUavHeapInfo.Heaps[i].Reset();
		}
	}

	D3D12DescriptorHeap::D3D12DescriptorAllocationInfo D3D12DescriptorHeap::Allocate(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t count)
	{
		switch (type)
		{
		case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV: return AllocateFromHeap(m_CbvSrvUavHeapInfo, type, count);
		case D3D12_DESCRIPTOR_HEAP_TYPE_RTV: return AllocateFromHeap(m_RtvHeapInfo, type, count);
		case D3D12_DESCRIPTOR_HEAP_TYPE_DSV: return AllocateFromHeap(m_DsvHeapInfo, type, count);
		default:GE_CORE_ASSERT(false, "Unknown descriptor heap type!");
			break;
		}
	}

	void D3D12DescriptorHeap::Free(const D3D12DescriptorAllocationInfo& info)
	{
		switch (info.Type)
		{
		case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV: FreeToHeap(m_CbvSrvUavHeapInfo, info); break;
		case D3D12_DESCRIPTOR_HEAP_TYPE_RTV: FreeToHeap(m_RtvHeapInfo, info); break;
		case D3D12_DESCRIPTOR_HEAP_TYPE_DSV: FreeToHeap(m_DsvHeapInfo, info); break;
		default: GE_CORE_ASSERT(false, "Unknown descriptor heap type!");
			break;
		}
	}

	D3D12DescriptorHeap::D3D12DescriptorAllocationInfo D3D12DescriptorHeap::AllocateFromHeap(DescriptorHeapInfo& heapInfo, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t count)
	{
		D3D12DescriptorAllocationInfo	allocationInfo{};
		allocationInfo.Type				= type;
		allocationInfo.Count			= count;
		allocationInfo.StartIndex		= UINT32_MAX;

		// look for free indices
		for (auto& index : heapInfo.FreeIndices)
		{
			for (int i = 0; i < count; i++)
			{
				// free space is not continuous, break and try next index
				if (index + i < heapInfo.DescriptorUsage.size() && heapInfo.DescriptorUsage[index + i] != 0)
				{
					break;
				}
				if(index + i >= heapInfo.DescriptorUsage.size())
				{
					break;
				}
				// find continuous free space, mark as used and return allocation info
				if (i == count - 1)
				{
					allocationInfo.StartIndex = index;
					break;
				}
			}
		}
		// if found free space, mark as used
		if (allocationInfo.StartIndex != UINT32_MAX)
		{
			for (int i = 0; i < count; i++)
			{
				heapInfo.DescriptorUsage[allocationInfo.StartIndex + i] = 1;
				heapInfo.FreeIndices.erase(allocationInfo.StartIndex + i);
			}
			// add the next index to free indices if it is free
			for(int i = allocationInfo.StartIndex + count; i < heapInfo.DescriptorUsage.size(); i++)
			{
				if (heapInfo.DescriptorUsage[i] == 0)
				{
					heapInfo.FreeIndices.insert(i);
					break;
				}
			}
			// add each frame's cpu and gpu handles to allocation info
			for(int i = 0; i < Graphics::GetFrameCount(); i++)
			{
				allocationInfo.CpuHandles.push_back(CD3DX12_CPU_DESCRIPTOR_HANDLE(heapInfo.Heaps[i].Get()->GetCPUDescriptorHandleForHeapStart(), allocationInfo.StartIndex, D3D12Context::Get()->GetCbvSrvUavDescriptorIncrementSize()));
				allocationInfo.GpuHandles.push_back(CD3DX12_GPU_DESCRIPTOR_HANDLE(heapInfo.Heaps[i].Get()->GetGPUDescriptorHandleForHeapStart(), allocationInfo.StartIndex, D3D12Context::Get()->GetCbvSrvUavDescriptorIncrementSize()));
			}
		}
		else
		{
			GE_CORE_ASSERT(false, "No free space in descriptor heap!");
		}
		return allocationInfo;
	}

	void D3D12DescriptorHeap::FreeToHeap(DescriptorHeapInfo& heapInfo, const D3D12DescriptorAllocationInfo& info)
	{
		if (info.IsValid() == false)
		{
			GE_CORE_ASSERT(false, "Invalid descriptor allocation info!");
			return;
		}
			
		// mark the descriptors as free
		for (int i = 0; i < info.Count; i++)
		{
			heapInfo.DescriptorUsage[info.StartIndex + i] = 0;
		}
		// looking front and back for free indices to merge
		uint32_t startIndex = info.StartIndex;
		// looking front
		for(int i = startIndex - 1; i >= 0; i--)
		{
			if (heapInfo.DescriptorUsage[i] == 0)
			{
				heapInfo.FreeIndices.erase(i);
				startIndex = i;
			}
			else
			{
				break;
			}
		}
		heapInfo.FreeIndices.insert(startIndex);
		// looking back
		startIndex = info.StartIndex + info.Count - 1;
		for(int i = startIndex; i < heapInfo.DescriptorUsage.size(); i++)
		{
			if (heapInfo.DescriptorUsage[i] == 0)
			{
				heapInfo.FreeIndices.erase(i);
				startIndex = i;
			}
			else
			{
				break;
			}
		}
		heapInfo.FreeIndices.insert(startIndex);
	}
}

