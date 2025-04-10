#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsContext.h"
#include <windows.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <directx/d3dx12.h>
#include "D3D12CommandBuffer.h"
#include "GEngine/Math/Math.h"

namespace GEngine
{
	class GENGINE_API D3D12Context : public  GraphicsContext
	{
	public:
		D3D12Context(HWND windowHandle);
		virtual ~D3D12Context() override;

		virtual void								Init(const unsigned int width, const unsigned int height) override;
		virtual void								Uninit() override;
		virtual void								SwapBuffers() override;
		virtual void								SetVSync(bool enable) override;
		virtual void								SetRequiredExtensions(std::vector<const char*> extensions) override;

		static D3D12Context*								Get() { return s_ContextInstance; }
		Microsoft::WRL::ComPtr<IDXGIFactory7>				GetFactory() { return m_Factory; }
		Microsoft::WRL::ComPtr<ID3D12Device>				GetDevice() { return m_Device; }
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>			GetGraphicsQueue() { return m_GraphicsQueue; }
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>			GetComputeQueue() { return m_ComputeQueue; }
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>			GetTransferQueue() { return m_TransferQueue; }
		Microsoft::WRL::ComPtr<IDXGISwapChain4>				GetSwapChain() { return m_SwapChain; }
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	BeginSingleTimeGraphicsCommand();
		UINT												GetRtvDescriptorSize() { return m_RtvDescriptorSize; }
		UINT												GetDsvDescriptorSize() { return m_DsvDescriptorSize; }
		UINT												GetCbvSrvUavDescriptorSize() { return m_CbvSrvUavDescriptorSize; }
		void												EndSingleTimeGraphicsCommand(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);
		Vector4&											GetClearColor() { return m_ClearColor; }
	protected:
		Ref<D3D12CommandBuffer>		GetCommandBuffer(CommandBufferType type);
	private:
		void						GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false);
		void						CreateFactory();
		void						CreateDevice();
		void						CreateQueues();
		void						CreateSwapChain(const unsigned int width, const unsigned int height);
		void						CreateCommandAllocator();
		void						CreateDescriptorHeap();
		void						CreateFrameResources();
		void						GetDescriptorSize();
	private:
		HWND											m_WindowHandle;
		static D3D12Context*							s_ContextInstance;
		Microsoft::WRL::ComPtr<IDXGIFactory7>			m_Factory;
		Microsoft::WRL::ComPtr<ID3D12Device>			m_Device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>		m_GraphicsQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>		m_ComputeQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>		m_TransferQueue;
		Microsoft::WRL::ComPtr<IDXGISwapChain4>			m_SwapChain;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_RtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_DsvHeap;
		UINT											m_RtvDescriptorSize;
		UINT											m_DsvDescriptorSize;
		UINT											m_CbvSrvUavDescriptorSize;
		D3D12CommandPool								m_CommandPool;

		uint32_t										m_SwapChainWidth, m_SwapChainHeight;
		bool											m_UseWarpDevice;

		Vector4											m_ClearColor = { 0, 0, 0, 0 };
	};
}


