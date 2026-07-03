#include "GEpch.h"
#include "D3D12Buffer.h"
#include "Platform/D3D12/D3D12Utils.h"
#include "Platform/D3D12/D3D12Context.h"

namespace GEngine
{
	D3D12VertexBuffer::D3D12VertexBuffer(uint32_t size, uint32_t sizeInstance, VertexTopology type)
	{
		m_TopologyType = type;
		Utils::CreateBuffer(
			size,
			D3D12_HEAP_TYPE_UPLOAD,
			D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			m_VertexBuffer
		);
		if (sizeInstance > 0)
		{
			m_InstanceRendering = true;
			Utils::CreateBuffer(
				sizeInstance,
				D3D12_HEAP_TYPE_UPLOAD,
				D3D12_HEAP_FLAG_NONE,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				m_InstanceBuffer
			);
		}
	}

	D3D12VertexBuffer::D3D12VertexBuffer(float* vertices, uint32_t size, uint32_t sizeInstance, VertexTopology type)
	{
		m_TopologyType = type;
		Utils::CreateBuffer(
			size,
			D3D12_HEAP_TYPE_UPLOAD,
			D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			m_VertexBuffer
		);
		if (sizeInstance > 0)
		{
			m_InstanceRendering = true;
			Utils::CreateBuffer(
				sizeInstance,
				D3D12_HEAP_TYPE_UPLOAD,
				D3D12_HEAP_FLAG_NONE,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				m_InstanceBuffer
			);
		}
		SetData(vertices, size);
	}

	D3D12VertexBuffer::~D3D12VertexBuffer()
	{
		m_VertexBuffer.Reset();
		m_InstanceBuffer.Reset();
	}

	void D3D12VertexBuffer::SetData(const void* data, uint32_t size)
	{
		void* mappedData;
		D3D12_RANGE readRange = { 0, 0 }; // We do not intend to read from this resource on the CPU.
		D3D12_THROW_IF_FAILED(m_VertexBuffer->Map(0, &readRange, &mappedData));
		memcpy(mappedData, data, size);
		m_VertexBuffer->Unmap(0, nullptr);
	}

	void D3D12VertexBuffer::SetDataInstance(const void* data, uint32_t size)
	{
		void* mappedData;
		D3D12_RANGE readRange = { 0, 0 }; // We do not intend to read from this resource on the CPU.
		D3D12_THROW_IF_FAILED(m_InstanceBuffer->Map(0, &readRange, &mappedData));
		memcpy(mappedData, data, size);
		m_InstanceBuffer->Unmap(0, nullptr);
	}
}
