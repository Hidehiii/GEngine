#include "GEpch.h"
#include "Platform/D3D12/D3D12UniformBuffer.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/D3D12/D3D12Utils.h"

namespace GEngine
{
	D3D12UniformBuffer::D3D12UniformBuffer(uint32_t size, uint32_t count, bool autoSetDataDynamic)
	{
		m_AutoSetDataDynamic = autoSetDataDynamic;
		const uint32_t minCbvAlignment = Graphics::GetMinUniformBufferOffsetAlignment();
		m_Aligment = (size + minCbvAlignment - 1) & ~(minCbvAlignment - 1);
		const uint32_t allocationSize = count > 1 ? m_Aligment * count : m_Aligment;

		if (count > 1)
		{
			m_TotalSize = allocationSize;
		}

		Utils::CreateBuffer(
			allocationSize,
			D3D12_HEAP_TYPE_UPLOAD,
			D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			m_ConstantBuffer
		);

		CD3DX12_RANGE noRead(0, 0);
		m_ConstantBuffer->Map(0, &noRead, reinterpret_cast<void**>(&m_MappedData));

		m_ConstantBufferViewDesc.BufferLocation = m_ConstantBuffer->GetGPUVirtualAddress();
		m_ConstantBufferViewDesc.SizeInBytes = m_Aligment;
	}
	D3D12UniformBuffer::~D3D12UniformBuffer()
	{
		if (m_ConstantBuffer)
		{
			m_ConstantBuffer->Unmap(0, nullptr);
			m_ConstantBuffer.Reset();
		}
	}
	void D3D12UniformBuffer::SetData(const void* data, uint32_t size)
	{
		GE_CORE_ASSERT(size <= m_Aligment, "Constant buffer data exceeds its aligned allocation.");
		memcpy(((std::byte*)m_MappedData) + m_Offset, data, size);

		// not sure apply offset now or in the command buffer, but apply offset now for now
		m_ConstantBufferViewDesc.BufferLocation = m_ConstantBuffer->GetGPUVirtualAddress() + m_Offset;
		m_ConstantBufferViewDesc.SizeInBytes	= m_Aligment;
	}
	void D3D12UniformBuffer::SetDataDynamic(const void* data, uint32_t size)
	{
		// if this is a dynamic uniform buffer, we need to update the offset for each write,
		// so that we can write to the next uniform buffer in the dynamic uniform buffer
		if (m_TotalSize > 0)
		{
			GE_CORE_ASSERT(size <= m_Aligment, "over size!");
			m_Offset = m_Offset + m_Aligment < m_TotalSize ? m_Offset + m_Aligment : 0;
		}

		memcpy(((std::byte*)m_MappedData) + m_Offset, data, size);

		// not sure apply offset now or in the command buffer, but apply offset now for now
		m_ConstantBufferViewDesc.BufferLocation = m_ConstantBuffer->GetGPUVirtualAddress() + m_Offset;
		m_ConstantBufferViewDesc.SizeInBytes	= m_Aligment;
	}
}

