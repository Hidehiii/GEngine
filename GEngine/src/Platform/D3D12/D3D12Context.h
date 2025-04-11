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
		UINT												GetBackBufferIndex() { return m_SwapChain->GetCurrentBackBufferIndex(); }

		std::pair<Microsoft::WRL::ComPtr<ID3D12Fence>, uint64_t>  GetFence();
	protected:
		Ref<D3D12CommandBuffer>		GetCommandBuffer(CommandBufferType type);
	private:
		void						GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false);
		void						CreateFactory();
		void						CreateDevice();
		void						CreateQueues();
		void						CreateSwapChain(const unsigned int width, const unsigned int height);
		void						CreateCommandAllocator();
		void						CreateFrameResources();
		void						GetDescriptorSize();
		void						CreateRenderPass();
		void						CreateRenderTargets();
		void						CreateFences();
	private:
		HWND											m_WindowHandle;
		static D3D12Context*							s_ContextInstance;
		Microsoft::WRL::ComPtr<IDXGIFactory7>			m_Factory;
		Microsoft::WRL::ComPtr<ID3D12Device>			m_Device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>		m_GraphicsQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>		m_ComputeQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>		m_TransferQueue;
		Microsoft::WRL::ComPtr<IDXGISwapChain4>			m_SwapChain;
		UINT											m_RtvDescriptorSize;
		UINT											m_DsvDescriptorSize;
		UINT											m_CbvSrvUavDescriptorSize;
		D3D12CommandPool								m_CommandPool;
		Ref<D3D12RenderPass>							m_SwapChainRenderPass;
		uint32_t										m_SwapChainWidth, m_SwapChainHeight;
		bool											m_UseWarpDevice;
		DXGI_FORMAT										m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		RenderPassSpecificationForD3D12					m_RenderPassSpec;
		std::vector<Ref<D3D12FrameBuffer>>				m_RenderTargets;
		int												m_Samples = 1;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Fence>>	m_Fences;
		size_t												m_FenceIndex = 0;
		uint64_t											m_FenceValue;

		Vector4											m_ClearColor = { 0, 0, 0, 0 };

		friend class D3D12GraphicsAPI;
	};
}


