#include "GEpch.h"
#include "D3D12Context.h"
#include "D3D12Utils.h"
#include "GEngine/Graphics/Graphics.h"

namespace GEngine
{
	D3D12Context* D3D12Context::s_ContextInstance = nullptr;

	D3D12Context::D3D12Context(HWND windowHandle)
	{
		m_WindowHandle = windowHandle;
		s_ContextInstance = this;
	}
	D3D12Context::~D3D12Context()
	{
	}
	void D3D12Context::Init(const unsigned int width, const unsigned int height)
	{
		CreateFactory();
		CreateDevice();
		CreateQueues();
		CreateSwapChain(width, height);
		GetDescriptorSize();
		CreateCommandAllocator();
	}
	void D3D12Context::Uninit()
	{
	}
	void D3D12Context::SwapBuffers()
	{
	}
	void D3D12Context::SetVSync(bool enable)
	{
	}
	void D3D12Context::SetRequiredExtensions(std::vector<const char*> extensions)
	{
	}
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> D3D12Context::BeginSingleTimeGraphicsCommand()
	{
		return m_CommandPool.BeginSingleTimeGraphicsCommand();
	}
	void D3D12Context::EndSingleTimeGraphicsCommand(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
	{
		m_CommandPool.EndSingleTimeGraphicsCommand(commandList);
	}
	Ref<D3D12CommandBuffer> D3D12Context::GetCommandBuffer(CommandBufferType type)
	{
		return m_CommandPool.GetCommandBuffer(type);
	}
	void D3D12Context::GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter)
	{
		*ppAdapter = nullptr;

		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;

		Microsoft::WRL::ComPtr<IDXGIFactory6> factory6;
		if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
		{
			for (
				UINT adapterIndex = 0;
				SUCCEEDED(factory6->EnumAdapterByGpuPreference(
					adapterIndex,
					requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
					IID_PPV_ARGS(&adapter)));
				++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter.
					// If you want a software adapter, pass in "/warp" on the command line.
					continue;
				}

				// Check to see whether the adapter supports Direct3D 12, but don't create the
				// actual device yet.
				if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
				{
					break;
				}
			}
		}

		if (adapter.Get() == nullptr)
		{
			for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter.
					// If you want a software adapter, pass in "/warp" on the command line.
					continue;
				}

				// Check to see whether the adapter supports Direct3D 12, but don't create the
				// actual device yet.
				if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
				{
					break;
				}
			}
		}

		*ppAdapter = adapter.Detach();
	}
	void D3D12Context::CreateFactory()
	{
		UINT dxgiFactoryFlags = 0;
#ifdef GE_DEBUG
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		D3D12_THROW_IF_FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
		D3D12_THROW_IF_FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_Factory)));
	}
	void D3D12Context::CreateDevice()
	{
		if (m_UseWarpDevice)
		{
			Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
			D3D12_THROW_IF_FAILED(m_Factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
			D3D12_THROW_IF_FAILED(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&m_Device)));
		}
		else
		{
			Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
			GetHardwareAdapter(m_Factory.Get(), &hardwareAdapter);
			D3D12_THROW_IF_FAILED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&m_Device)));
		}
	}
	void D3D12Context::CreateQueues()
	{
		D3D12_COMMAND_QUEUE_DESC	queueDesc = {};
		queueDesc.Flags				= D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type				= D3D12_COMMAND_LIST_TYPE_DIRECT;

		D3D12_THROW_IF_FAILED(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_GraphicsQueue)));

		queueDesc.Type				= D3D12_COMMAND_LIST_TYPE_COMPUTE;
		D3D12_THROW_IF_FAILED(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_ComputeQueue)));

		queueDesc.Type				= D3D12_COMMAND_LIST_TYPE_COPY;
		D3D12_THROW_IF_FAILED(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_TransferQueue)));
	}
	void D3D12Context::CreateSwapChain(const unsigned int width, const unsigned int height)
	{
		m_SwapChainWidth = width;
		m_SwapChainHeight = height;

		DXGI_SWAP_CHAIN_DESC1			swapChainDesc = {};
		swapChainDesc.BufferCount		= Graphics::GetFrameCount();
		swapChainDesc.Width				= width;
		swapChainDesc.Height			= height;
		swapChainDesc.Format			= DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage		= DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect		= DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count	= 1;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC		fullscreenDesc{};
		fullscreenDesc.RefreshRate			= { 0, 0 };
		fullscreenDesc.ScanlineOrdering		= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		fullscreenDesc.Scaling				= DXGI_MODE_SCALING_UNSPECIFIED;
		fullscreenDesc.Windowed				= TRUE;

		Microsoft::WRL::ComPtr<IDXGISwapChain1>	swapChain;
		D3D12_THROW_IF_FAILED(m_Factory->CreateSwapChainForHwnd(m_GraphicsQueue.Get(), m_WindowHandle, &swapChainDesc, &fullscreenDesc, nullptr, &swapChain));
		D3D12_THROW_IF_FAILED(swapChain.As(&m_SwapChain));
	}
	void D3D12Context::CreateCommandAllocator()
	{
		m_CommandPool = D3D12CommandPool();
	}
	void D3D12Context::CreateDescriptorHeap()
	{
		D3D12_DESCRIPTOR_HEAP_DESC		rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors		= Graphics::GetFrameCount();
		rtvHeapDesc.Type				= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags				= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		D3D12_THROW_IF_FAILED(m_Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RtvHeap)));

		D3D12_DESCRIPTOR_HEAP_DESC		rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors		= Graphics::GetFrameCount();
		rtvHeapDesc.Type				= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags				= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	}
	void D3D12Context::CreateFrameResources()
	{
	}
	void D3D12Context::GetDescriptorSize()
	{
		m_RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_DsvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_CbvSrvUavDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
}