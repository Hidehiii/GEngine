#include "GEpch.h"
#include "D3D12StorageBuffer.h"
#include "Platform/D3D12/D3D12Context.h"
#include "Platform/D3D12/D3D12Utils.h"

namespace GEngine
{
	D3D12StorageBuffer::D3D12StorageBuffer(uint32_t size) : m_Size(size)
	{
		GE_CORE_ASSERT(size > 0, "Storage buffer size must be non-zero.");
		D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &desc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_Resource)));

		m_UavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		m_UavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		m_UavDesc.Buffer.FirstElement = 0;
		m_UavDesc.Buffer.NumElements = size / sizeof(uint32_t);
		m_UavDesc.Buffer.StructureByteStride = 0;
		m_UavDesc.Buffer.CounterOffsetInBytes = 0;
		m_UavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
	}

	void D3D12StorageBuffer::SetData(uint32_t size, const void* data, uint32_t offset)
	{
		GE_CORE_ASSERT(data && size > 0 && offset + size <= m_Size, "Storage buffer write is outside its allocation.");
		Microsoft::WRL::ComPtr<ID3D12Resource> upload;
		Utils::CreateBuffer(size, D3D12_HEAP_TYPE_UPLOAD, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, upload);

		void* mapped = nullptr;
		D3D12_RANGE readRange{ 0, 0 };
		D3D12_THROW_IF_FAILED(upload->Map(0, &readRange, &mapped));
		memcpy(mapped, data, size);
		upload->Unmap(0, nullptr);

		auto command = D3D12Context::Get()->BeginSingleTimeGraphicsCommand();
		auto commandList = command->GetCommandList();
		CD3DX12_RESOURCE_BARRIER toCopy = CD3DX12_RESOURCE_BARRIER::Transition(m_Resource.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);
		commandList->ResourceBarrier(1, &toCopy);
		commandList->CopyBufferRegion(m_Resource.Get(), offset, upload.Get(), 0, size);
		CD3DX12_RESOURCE_BARRIER toUav = CD3DX12_RESOURCE_BARRIER::Transition(m_Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandList->ResourceBarrier(1, &toUav);
		D3D12Context::Get()->EndSingleTimeGraphicsCommand(command);
	}
}
