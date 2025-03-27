#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsContext.h"
#include <windows.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <directx/d3dx12.h>
#include "D3D12CommandBuffer.h"

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
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>			GetCommandQueue() { return m_CommandQueue; }
		Microsoft::WRL::ComPtr<IDXGISwapChain4>				GetSwapChain() { return m_SwapChain; }
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	BeginSingleTimeGraphicsCommand();
		void												EndSingleTimeGraphicsCommand(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);
	private:
		void						GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false);
		void						CreateFactory();
		void						CreateDevice();
		void						CreateCommandQueue();
		void						CreateSwapChain(const unsigned int width, const unsigned int height);
		void						CreateCommandAllocator();
		void						CreateDescriptorHeap();
		void						CreateFrameResources();
	private:
		HWND											m_WindowHandle;
		static D3D12Context*							s_ContextInstance;
		Microsoft::WRL::ComPtr<IDXGIFactory7>			m_Factory;
		Microsoft::WRL::ComPtr<ID3D12Device>			m_Device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>		m_CommandQueue;
		Microsoft::WRL::ComPtr<IDXGISwapChain4>			m_SwapChain;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_RtvHeap;
		D3D12CommandAllocator							m_CommandAllocator;

		uint32_t										m_SwapChainWidth, m_SwapChainHeight;
		bool											m_UseWarpDevice;
	};
}


