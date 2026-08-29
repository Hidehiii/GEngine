#pragma once

#include "GEngine/Compute/StorageBuffer.h"
#include <d3d12.h>
#include <wrl.h>

namespace GEngine
{
	class GENGINE_API D3D12StorageBuffer : public StorageBuffer
	{
	public:
		explicit D3D12StorageBuffer(uint32_t size);
		virtual ~D3D12StorageBuffer() override = default;

		virtual void SetData(uint32_t size, const void* data, uint32_t offset = 0) override;
		virtual void Bind(uint32_t slot) override {}

		ID3D12Resource* GetResource() const { return m_Resource.Get(); }
		const D3D12_UNORDERED_ACCESS_VIEW_DESC& GetUnorderedAccessViewDesc() const { return m_UavDesc; }

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_Resource;
		D3D12_UNORDERED_ACCESS_VIEW_DESC m_UavDesc{};
		uint32_t m_Size = 0;
	};
}
