#pragma once
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <directx/d3dx12.h>

inline std::string HrToString(HRESULT hr)
{
	char s_str[64] = {};
	sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
	return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
	HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
	HRESULT Error() const { return m_hr; }
private:
	const HRESULT m_hr;
};

#define D3D12_THROW_IF_FAILED(f)																			\
{																											\
	HRESULT	res = (f);																						\
	if (FAILED(res))																						\
	{																										\
		throw HrException(res);																				\
	}																										\
}

namespace GEngine
{
	namespace Utils
	{
		void CreateTextures(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevel, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS resourceFlag, D3D12_HEAP_FLAGS heapFlag, D3D12_RESOURCE_DIMENSION dimesion, D3D12_RESOURCE_STATES state, Microsoft::WRL::ComPtr<ID3D12Resource>& texture);
		void CreateBuffers(uint32_t size, D3D12_HEAP_TYPE heapType, D3D12_HEAP_FLAGS flag, D3D12_RESOURCE_STATES state, Microsoft::WRL::ComPtr<ID3D12Resource>& buffer);

		void CopyDataToTextures(uint32_t width, uint32_t height, uint32_t pixelSize, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, void* data);
	}
}

