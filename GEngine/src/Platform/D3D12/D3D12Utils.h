#pragma once
#include <windows.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <directx/d3dx12.h>
#include "GEngine/Graphics/FrameBuffer.h"

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
		void CreateTexture(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevel, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS resourceFlag, D3D12_HEAP_FLAGS heapFlag, D3D12_RESOURCE_DIMENSION dimesion, D3D12_RESOURCE_STATES state, Microsoft::WRL::ComPtr<ID3D12Resource>& texture);
		void CreateBuffer(uint32_t size, D3D12_HEAP_TYPE heapType, D3D12_HEAP_FLAGS flag, D3D12_RESOURCE_STATES state, Microsoft::WRL::ComPtr<ID3D12Resource>& buffer);

		void CopyDataToTextures(uint32_t width, uint32_t height, uint32_t pixelSize, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const void* data);

		void TransitionResourceState(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, Microsoft::WRL::ComPtr<ID3D12Resource>& resource, D3D12_RESOURCE_STATES src, D3D12_RESOURCE_STATES dst);
		void TransitionResourceState(Microsoft::WRL::ComPtr<ID3D12Resource>& resource, D3D12_RESOURCE_STATES src, D3D12_RESOURCE_STATES dst);

		void CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags);

		DXGI_FORMAT					FrameBufferTextureFormatToDXGIFormat(FrameBufferTextureFormat format);
		FrameBufferTextureFormat	DXGIFormatToFrameBufferTextureFormat(DXGI_FORMAT format);
		DXGI_FORMAT					ShaderInputDataTypeToDXGIFormat(ShaderInputDataType type);
		DXGI_FORMAT					RenderImage2DFormatToDXGIFormat(RenderImage2DFormat format);
		D3D12_CULL_MODE				CullModeToD3D12CullMode(CullMode mode);
		D3D12_BLEND					BlendFactorToD3D12Blend(BlendFactor factor);
		D3D12_BLEND_OP				BlendModeToD3D12BlendOp(BlendMode mode);
		D3D12_COMPARISON_FUNC		CompareOperationToD3D12ComparisonFunc(CompareOperation op);
		D3D12_PRIMITIVE_TOPOLOGY	VertexTopologyToD3D12PrimitiveTopology(VertexTopology type);
	}
}

