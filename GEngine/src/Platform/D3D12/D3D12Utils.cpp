#include "GEpch.h"
#include "D3D12Utils.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/D3D12/D3D12Context.h"

namespace GEngine
{
	namespace Utils
	{
		void CreateTextures(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevel, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS resourceFlag, D3D12_HEAP_FLAGS heapFlag, D3D12_RESOURCE_DIMENSION dimesion, D3D12_RESOURCE_STATES state, Microsoft::WRL::ComPtr<ID3D12Resource>& texture)
		{
			D3D12_RESOURCE_DESC				textureDesc = {};
			textureDesc.MipLevels			= mipLevel;
			textureDesc.Format				= format;
			textureDesc.Width				= width;
			textureDesc.Height				= height;
			textureDesc.Flags				= resourceFlag;
			textureDesc.DepthOrArraySize	= depth;
			textureDesc.SampleDesc.Count	= 1;
			textureDesc.SampleDesc.Quality	= 0;
			textureDesc.Dimension			= dimesion;

			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				heapFlag,
				&textureDesc,
				state,
				nullptr,
				IID_PPV_ARGS(&texture)
			));
		}
		void CreateBuffers(uint32_t size, D3D12_HEAP_TYPE heapType, D3D12_HEAP_FLAGS flag, D3D12_RESOURCE_STATES state, Microsoft::WRL::ComPtr<ID3D12Resource>& buffer)
		{
			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(heapType),
				flag,
				&CD3DX12_RESOURCE_DESC::Buffer(size),
				state,
				nullptr,
				IID_PPV_ARGS(&buffer)
			));
		}
		void CopyDataToTextures(uint32_t width, uint32_t height, uint32_t pixelSize, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const void* data)
		{
			D3D12_SUBRESOURCE_DATA		textureData{};
			textureData.pData			= data;
			textureData.RowPitch		= width * pixelSize;
			textureData.SlicePitch		= textureData.RowPitch * height;

			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = D3D12Context::Get()->BeginSingleTimeGraphicsCommand();

			UpdateSubresources(commandList.Get(), texture.Get(), uploadBuffer.Get(), 0, 0, 1, &textureData);

			D3D12Context::Get()->EndSingleTimeGraphicsCommand(commandList);
		}
		void TransitionResourceState(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, Microsoft::WRL::ComPtr<ID3D12Resource>& resource, D3D12_RESOURCE_STATES src, D3D12_RESOURCE_STATES dst)
		{
			if(src != dst)
				commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), src, dst));
		}
		void TransitionResourceState(Microsoft::WRL::ComPtr<ID3D12Resource>& resource, D3D12_RESOURCE_STATES src, D3D12_RESOURCE_STATES dst)
		{
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = D3D12Context::Get()->BeginSingleTimeGraphicsCommand();
			TransitionResourceState(commandList, resource, src, dst);
			D3D12Context::Get()->EndSingleTimeGraphicsCommand(commandList);
		}
		void SetFenceValue(Microsoft::WRL::ComPtr<ID3D12CommandQueue>& queue, Microsoft::WRL::ComPtr<ID3D12Fence>& fence, const UINT64& value, HANDLE& event)
		{
			D3D12_THROW_IF_FAILED(queue->Signal(fence.Get(), value));
			if (fence->GetCompletedValue() != value)
			{
				D3D12_THROW_IF_FAILED(fence->SetEventOnCompletion(value, event));
				WaitForSingleObject(event, INFINITE);
			}
		}
		void WaitForFence(Microsoft::WRL::ComPtr<ID3D12CommandQueue>& queue, Microsoft::WRL::ComPtr<ID3D12Fence>& fence, const UINT64& value, HANDLE& event)
		{
			if (fence->GetCompletedValue() < value)
			{
				D3D12_THROW_IF_FAILED(fence->SetEventOnCompletion(value, event));
				WaitForSingleObject(event, INFINITE);
			}
		}
		HANDLE& CreateFenceEvent(LPSECURITY_ATTRIBUTES IpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCWSTR IpName)
		{
			HANDLE event = CreateEvent(IpEventAttributes, bManualReset, bInitialState, IpName);
			if (event == nullptr)
			{
				D3D12_THROW_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
			}
			return event;
		}
		DXGI_FORMAT FrameBufferTextureFormatToDXGIFormat(FrameBufferTextureFormat format)
		{
			switch (format)
			{
			case FrameBufferTextureFormat::None:
				return DXGI_FORMAT_UNKNOWN;
			case FrameBufferTextureFormat::RGBA8:
				return DXGI_FORMAT_R8G8B8A8_UNORM;
			case FrameBufferTextureFormat::R32F:
				return DXGI_FORMAT_R32_FLOAT;
			case FrameBufferTextureFormat::RG16F:
				return DXGI_FORMAT_R16G16_FLOAT;
			case FrameBufferTextureFormat::R32I:
				return DXGI_FORMAT_R32_SINT;
			case FrameBufferTextureFormat::RG16I:
				return DXGI_FORMAT_R16G16_SINT;
			case FrameBufferTextureFormat::R32UI:
				return DXGI_FORMAT_R32_UINT;
			case FrameBufferTextureFormat::RG16UI:
				return DXGI_FORMAT_R16G16_UINT;
			case FrameBufferTextureFormat::DEPTH24STENCIL8:
				return DXGI_FORMAT_D24_UNORM_S8_UINT;
			case FrameBufferTextureFormat::DEPTH:
				return DXGI_FORMAT_D32_FLOAT;
			default:
				break;
			}
			return DXGI_FORMAT();
		}
		FrameBufferTextureFormat DXGIFormatToFrameBufferTextureFormat(DXGI_FORMAT format)
		{
			switch (format)
			{
			case DXGI_FORMAT_UNKNOWN:			return FrameBufferTextureFormat::None;
			case DXGI_FORMAT_R8G8B8A8_UNORM:	return FrameBufferTextureFormat::RGBA8;
			case DXGI_FORMAT_R32_FLOAT:			return FrameBufferTextureFormat::R32F;
			case DXGI_FORMAT_R16G16_FLOAT:		return FrameBufferTextureFormat::RG16F;
			case DXGI_FORMAT_R32_SINT:			return FrameBufferTextureFormat::R32I;
			case DXGI_FORMAT_R16G16_SINT:		return FrameBufferTextureFormat::RG16I;
			case DXGI_FORMAT_R32_UINT:			return FrameBufferTextureFormat::R32UI
			case DXGI_FORMAT_R16G16_UINT:		return FrameBufferTextureFormat::RG16UI;
			case DXGI_FORMAT_D24_UNORM_S8_UINT:	return FrameBufferTextureFormat::DEPTH24STENCIL8;
			case DXGI_FORMAT_D32_FLOAT:			return FrameBufferTextureFormat::DEPTH;
			default:
				break;
			}
			return FrameBufferTextureFormat();
		}
	}
}