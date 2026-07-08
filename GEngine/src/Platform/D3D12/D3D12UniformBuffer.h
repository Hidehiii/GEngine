#pragma once
#include "GEngine/Graphics/UniformBuffer.h"
#include <windows.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <directx/d3dx12.h>


namespace GEngine
{
	class GENGINE_API D3D12UniformBuffer : public UniformBuffer
	{
	public:
		D3D12UniformBuffer(uint32_t size, uint32_t count = 1);
		virtual ~D3D12UniformBuffer();

		virtual void SetData(const void* data, uint32_t size) override;

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource>			m_ConstantBuffer = nullptr;
		D3D12_CONSTANT_BUFFER_VIEW_DESC					m_ConstantBufferViewDesc{};
		void*											m_MappedData = nullptr;		
	};
}


