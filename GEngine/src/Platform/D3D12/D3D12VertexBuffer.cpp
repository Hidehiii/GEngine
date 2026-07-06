#include "GEpch.h"
#include "D3D12VertexBuffer.h"
#include "Platform/D3D12/D3D12Utils.h"
#include "Platform/D3D12/D3D12Context.h"

namespace GEngine
{
	D3D12VertexBuffer::D3D12VertexBuffer(uint32_t size, uint32_t sizeInstance, VertexTopology type)
	{
		m_TopologyType	= type;
		m_TotalSizeVertex	= size;

		Utils::CreateBuffer(
			size,
			D3D12_HEAP_TYPE_UPLOAD,
			D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			m_VertexBuffer
		);

		m_VertexBufferView.BufferLocation	= m_VertexBuffer->GetGPUVirtualAddress();
		m_VertexBufferView.SizeInBytes		= size;

		if (sizeInstance > 0)
		{
			m_TotalSizeInstance		= sizeInstance;

			Utils::CreateBuffer(
				sizeInstance,
				D3D12_HEAP_TYPE_UPLOAD,
				D3D12_HEAP_FLAG_NONE,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				m_InstanceBuffer
			);

			m_InstanceBufferView.BufferLocation = m_InstanceBuffer->GetGPUVirtualAddress();
			m_InstanceBufferView.SizeInBytes	= sizeInstance;
		}
	}

	D3D12VertexBuffer::D3D12VertexBuffer(const void* vertices, uint32_t size, uint32_t sizeInstance, VertexTopology type)
	{
		m_TopologyType	= type;
		m_TotalSizeVertex	= size;

		Utils::CreateBuffer(
			size,
			D3D12_HEAP_TYPE_UPLOAD,
			D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			m_VertexBuffer
		);

		m_VertexBufferView.BufferLocation	= m_VertexBuffer->GetGPUVirtualAddress();
		m_VertexBufferView.SizeInBytes		= size;

		if (sizeInstance > 0)
		{
			m_TotalSizeInstance		= sizeInstance;

			Utils::CreateBuffer(
				sizeInstance,
				D3D12_HEAP_TYPE_UPLOAD,
				D3D12_HEAP_FLAG_NONE,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				m_InstanceBuffer
			);

			m_InstanceBufferView.BufferLocation = m_InstanceBuffer->GetGPUVirtualAddress();
			m_InstanceBufferView.SizeInBytes	= sizeInstance;
		}
		SetVertexData(vertices, size);
	}

	D3D12VertexBuffer::~D3D12VertexBuffer()
	{
		m_VertexBuffer.Reset();
		m_InstanceBuffer.Reset();
	}

	void D3D12VertexBuffer::SetVertexData(const void* data, uint32_t size)
	{
		void* mappedData;
		D3D12_RANGE readRange = { 0, 0 }; // We do not intend to read from this resource on the CPU.
		D3D12_THROW_IF_FAILED(m_VertexBuffer->Map(0, &readRange, &mappedData));
		memcpy(mappedData, data, size);
		m_VertexBuffer->Unmap(0, nullptr);
	}

	void D3D12VertexBuffer::SetInstanceData(const void* data, uint32_t size)
	{
		GE_CORE_ASSERT(m_InstanceBuffer, "Instance rendering is not enabled for this vertex buffer.");
		void* mappedData;
		D3D12_RANGE readRange = { 0, 0 }; // We do not intend to read from this resource on the CPU.
		D3D12_THROW_IF_FAILED(m_InstanceBuffer->Map(0, &readRange, &mappedData));
		memcpy(mappedData, data, size);
		m_InstanceBuffer->Unmap(0, nullptr);
	}

	void D3D12VertexBuffer::SetIndexBuffer(const Ref<GEngine::IndexBuffer>& indexBuffer)
	{
		m_IndexBuffer = std::static_pointer_cast<D3D12IndexBuffer>(indexBuffer);
	}


	uint32_t D3D12VertexBuffer::GetIndexCount() const
	{
		return m_IndexBuffer != nullptr ? m_IndexBuffer->GetCount() : 0;
	}

	void D3D12VertexBuffer::Bind(CommandBuffer* cmd) const
	{
		auto list = static_cast<D3D12CommandBuffer*>(cmd)->GetCommandList();
		if (m_InstanceBuffer)
		{
			D3D12_VERTEX_BUFFER_VIEW views[] = { m_VertexBufferView, m_InstanceBufferView };
			list->IASetVertexBuffers(0, 2, views);
		}
		else
		{
			list->IASetVertexBuffers(0, 1, &m_VertexBufferView);
		}
		if(m_IndexBuffer)
			m_IndexBuffer->Bind(cmd);
	}

	void D3D12VertexBuffer::SetShaderAndInputLayout(const Ref<Shader>& shader, uint32_t pass)
	{
		GE_CORE_ASSERT(shader, "Shader is null!");
		m_Shader = shader;
		m_ShaderPass = pass;

		m_InputElementDescs.clear();
		
		m_VertexBufferView.StrideInBytes = m_Shader->GetPassReflections().at(m_ShaderPass).VertexInputVertexStride;
		m_InstanceBufferView.StrideInBytes = m_Shader->GetPassReflections().at(m_ShaderPass).VertexInputInstanceStride;

		D3D12_INPUT_ELEMENT_DESC inputElementDesc = {};
		for (auto& e : m_Shader->GetPassReflections().at(m_ShaderPass).VertexInputs)
		{
			inputElementDesc.SemanticName			= e.SemanticName.c_str();
			inputElementDesc.SemanticIndex			= e.SemanticIndex;
			inputElementDesc.Format					= Utils::ShaderInputDataTypeToDXGIFormat(e.Type);
			inputElementDesc.InputSlot				= e.IsPerInstance ? 1 : 0;
			inputElementDesc.AlignedByteOffset		= e.Offset;
			inputElementDesc.InputSlotClass			= e.IsPerInstance ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			inputElementDesc.InstanceDataStepRate	= e.IsPerInstance ? 1 : 0;

			m_InputElementDescs.push_back(inputElementDesc);
		}
	}

	D3D12IndexBuffer::D3D12IndexBuffer(const uint32_t* indices, uint32_t count)
	{
		m_Count = count;
		Utils::CreateBuffer(
			count * sizeof(uint32_t),
			D3D12_HEAP_TYPE_UPLOAD,
			D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			m_IndexBuffer
		);

		m_IndexBufferView.BufferLocation	= m_IndexBuffer->GetGPUVirtualAddress();
		m_IndexBufferView.SizeInBytes		= count * sizeof(uint32_t);
		m_IndexBufferView.Format			= DXGI_FORMAT_R32_UINT;

		void* mappedData;
		D3D12_RANGE readRange = { 0, 0 }; // We do not intend to read from this resource on the CPU.
		D3D12_THROW_IF_FAILED(m_IndexBuffer->Map(0, &readRange, &mappedData));
		memcpy(mappedData, indices, count * sizeof(uint32_t));
		m_IndexBuffer->Unmap(0, nullptr);
	}

	D3D12IndexBuffer::~D3D12IndexBuffer()
	{
		m_IndexBuffer.Reset();
	}

	void D3D12IndexBuffer::Bind(CommandBuffer* cmd) const
	{
		auto list = static_cast<D3D12CommandBuffer*>(cmd)->GetCommandList();
		list->IASetIndexBuffer(&m_IndexBufferView);
	}

}
