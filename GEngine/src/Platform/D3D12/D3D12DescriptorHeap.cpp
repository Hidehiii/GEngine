#include "GEpch.h"
#include "Platform/D3D12/D3D12DescriptorHeap.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/D3D12/D3D12Utils.h"
#include "Platform/D3D12/D3D12Context.h"

namespace GEngine
{
	void D3D12DescriptorHeap::Initialize(uint32_t rtvCount, uint32_t dsvCount, uint32_t cbvSrvUavCount, uint32_t samplerCount)
	{
		GE_CORE_ASSERT(!m_Initialized, "Descriptor heap has already been initialized!");
		m_Initialized = true;

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

		m_SamplerHeapInfo.DescriptorCount = samplerCount;
		m_SamplerHeapInfo.DescriptorUsage = std::vector<uint8_t>(samplerCount, 0);
		m_SamplerHeapInfo.Heaps.resize(Graphics::GetFrameCount());
		m_SamplerHeapInfo.FreeIndices.insert(0);

		heapDesc.NumDescriptors = samplerCount;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		for (int i = 0; i < Graphics::GetFrameCount(); i++)
		{
			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_SamplerHeapInfo.Heaps[i])));
		}
	}

	D3D12DescriptorHeap::~D3D12DescriptorHeap()
	{
		// D3D12Context::Uninit synchronizes GPU work before member destruction.
		// Releasing ComPtr containers directly also keeps partially initialized contexts safe.
	}

	D3D12DescriptorHeap::D3D12DescriptorAllocationInfo D3D12DescriptorHeap::Allocate(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t count)
	{
		switch (type)
		{
		case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV: return AllocateFromHeap(m_CbvSrvUavHeapInfo, type, count);
		case D3D12_DESCRIPTOR_HEAP_TYPE_RTV: return AllocateFromHeap(m_RtvHeapInfo, type, count);
		case D3D12_DESCRIPTOR_HEAP_TYPE_DSV: return AllocateFromHeap(m_DsvHeapInfo, type, count);
		case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER: return AllocateFromHeap(m_SamplerHeapInfo, type, count);
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
		case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER: FreeToHeap(m_SamplerHeapInfo, info); break;
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

		if (count == 0)
			return allocationInfo;

		// Look for a contiguous free range.  The heap is shared by all frames, while
		// each allocation stores one CPU/GPU handle pair per frame.
		for (auto& index : heapInfo.FreeIndices)
		{
			for (uint32_t i = 0; i < count; i++)
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
			for (uint32_t i = 0; i < count; i++)
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
			const UINT descriptorIncrement = D3D12Context::Get()->GetDevice()->GetDescriptorHandleIncrementSize(type);
			for(int i = 0; i < Graphics::GetFrameCount(); i++)
			{
				allocationInfo.CpuHandles.push_back(CD3DX12_CPU_DESCRIPTOR_HANDLE(heapInfo.Heaps[i].Get()->GetCPUDescriptorHandleForHeapStart(), allocationInfo.StartIndex, descriptorIncrement));
				if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
					allocationInfo.GpuHandles.push_back(CD3DX12_GPU_DESCRIPTOR_HANDLE(heapInfo.Heaps[i].Get()->GetGPUDescriptorHandleForHeapStart(), allocationInfo.StartIndex, descriptorIncrement));
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
			
		GE_CORE_ASSERT(info.StartIndex + info.Count <= heapInfo.DescriptorCount, "Descriptor allocation is outside this heap!");
		for (uint32_t i = 0; i < info.Count; i++)
		{
			heapInfo.DescriptorUsage[info.StartIndex + i] = 0;
		}

		// Rebuild range starts.  This is rare (material/frame-buffer destruction) and
		// avoids the unsigned-underflow and stale-range bugs in the old merge code.
		heapInfo.FreeIndices.clear();
		for (uint32_t index = 0; index < heapInfo.DescriptorCount; ++index)
		{
			if (heapInfo.DescriptorUsage[index] == 0 && (index == 0 || heapInfo.DescriptorUsage[index - 1] != 0))
				heapInfo.FreeIndices.insert(index);
		}
	}
}

