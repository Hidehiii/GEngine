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
		void TransitionResourceStage(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, Microsoft::WRL::ComPtr<ID3D12Resource>& resource, D3D12_RESOURCE_STATES src, D3D12_RESOURCE_STATES dst)
		{
			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), src, dst));
		}
		void TransitionResourceStage(Microsoft::WRL::ComPtr<ID3D12Resource>& resource, D3D12_RESOURCE_STATES src, D3D12_RESOURCE_STATES dst)
		{
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = D3D12Context::Get()->BeginSingleTimeGraphicsCommand();
			TransitionResourceStage(commandList, resource, src, dst);
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
	}
}