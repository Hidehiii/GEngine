#pragma once

#include "GEngine/Compute/StorageImage.h"
#include <d3d12.h>
#include <wrl.h>

namespace GEngine
{
	class GENGINE_API D3D12StorageImage2D : public StorageImage2D
	{
	public:
		D3D12StorageImage2D(uint32_t width, uint32_t height, ComputeImage2DFormat format);
		virtual ~D3D12StorageImage2D() override = default;

		virtual void Bind(CommandBuffer* cmdBuffer, const uint32_t slot = 0) override {}

		ID3D12Resource* GetResource() const { return m_Resource.Get(); }
		const D3D12_UNORDERED_ACCESS_VIEW_DESC& GetUnorderedAccessViewDesc() const { return m_UavDesc; }

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_Resource;
		D3D12_UNORDERED_ACCESS_VIEW_DESC m_UavDesc{};
	};
}
