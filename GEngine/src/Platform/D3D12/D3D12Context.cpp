#include "GEpch.h"
#include "Platform/D3D12/D3D12Context.h"
#include "Platform/D3D12/D3D12Utils.h"
#include "GEngine/Core/Config.h"
#include "GEngine/Graphics/Graphics.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <cstdlib>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>


namespace GEngine
{
namespace
{
	bool IsD3D12DebugLayerRequested()
	{
		const char* value = std::getenv("GENGINE_D3D12_DEBUG_LAYER");
		return value != nullptr && value[0] == '1' && value[1] == '\0';
	}
}

#ifdef GE_DEBUG
	std::atomic_bool	g_D3D12DebugInfoQueueLoggerRunning{ false };
	std::thread			g_D3D12DebugInfoQueueLoggerThread;
	static void D3D12DebugInfoQueueLoggerThreadFunc(Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue)
	{
		while (g_D3D12DebugInfoQueueLoggerRunning.load())
		{
			UINT64 num = infoQueue->GetNumStoredMessagesAllowedByRetrievalFilter();
			for (UINT64 i = 0; i < num; ++i)
			{
				SIZE_T msgLen = 0;
				infoQueue->GetMessage(i, nullptr, &msgLen);
				std::vector<char> buf(msgLen);
				D3D12_MESSAGE* msg = reinterpret_cast<D3D12_MESSAGE*>(buf.data());
				infoQueue->GetMessage(i, msg, &msgLen);

				if (msg->Severity == D3D12_MESSAGE_SEVERITY_WARNING)
				{
					GE_CORE_WARN("D3D12 Warning: {0}", msg->pDescription ? msg->pDescription : "null");
				}
				if(msg->Severity == D3D12_MESSAGE_SEVERITY_ERROR || msg->Severity == D3D12_MESSAGE_SEVERITY_CORRUPTION)
				{
					GE_CORE_ERROR("D3D12 Error: {0}", msg->pDescription ? msg->pDescription : "null");
				}
			}
			// 清除已存消息
			infoQueue->ClearStoredMessages();
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
	static void StartD3D12DebugInfoQueueLogger(Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue)
	{
		if(g_D3D12DebugInfoQueueLoggerRunning.exchange(true))
		{
			return;
		}
		GE_CORE_INFO("Starting D3D12 debug info queue logger thread.");
		g_D3D12DebugInfoQueueLoggerThread = std::thread(D3D12DebugInfoQueueLoggerThreadFunc, infoQueue);
	}
	static void StopD3D12DebugInfoQueueLogger()
	{
		if(!g_D3D12DebugInfoQueueLoggerRunning.exchange(false))
		{
			return;
		}
		GE_CORE_INFO("Stopping D3D12 debug info queue logger thread.");
		if (g_D3D12DebugInfoQueueLoggerThread.joinable())
		{
			g_D3D12DebugInfoQueueLoggerThread.join();
		}
	}
#endif

	D3D12Context* D3D12Context::s_ContextInstance = nullptr;

	D3D12Context::D3D12Context(void* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		s_ContextInstance = this;
	}
	D3D12Context::~D3D12Context()
	{
		
	}
	void D3D12Context::Init(const unsigned int width, const unsigned int height)
	{
		CreateFactory();
		CreateDevice();
#ifdef GE_DEBUG
		if (IsD3D12DebugLayerRequested())
			SetupDebugInfoQueue();
#endif
		CreateQueues();
		CreateSwapChain(width, height);
		CreateDescriptorHeaps();
		GetDescriptorSize();
		CreateCommandAllocator();
		CreateRenderPass();
		CreateRenderTargets();
		CreateFences();
	}
	void D3D12Context::Uninit()
	{
		WaitForFence(COMMAND_BUFFER_TYPE_GRAPHICS);
		WaitForFence(COMMAND_BUFFER_TYPE_COMPUTE);
		WaitForFence(COMMAND_BUFFER_TYPE_TRANSFER);

		for(int i = 0; i < m_FenceEvents.size(); i++)
		{
			CloseHandle(m_FenceEvents[i]);
		}
		m_Fences.clear();

	}
	void D3D12Context::SetVSync(bool enable)
	{
		m_VSync = enable;
	}
	void D3D12Context::SetRequiredExtensions(std::vector<const char*> extensions)
	{
	}
	Ref<D3D12CommandBuffer> D3D12Context::BeginSingleTimeGraphicsCommand()
	{
		return m_CommandPool.BeginSingleTimeGraphicsCommand();
	}
	void D3D12Context::EndSingleTimeGraphicsCommand(Ref<D3D12CommandBuffer>& commandList)
	{
		m_CommandPool.EndSingleTimeGraphicsCommand(commandList);
	}
	Ref<D3D12CommandBuffer> D3D12Context::BeginSingleTimeComputeCommand()
	{
		return m_CommandPool.BeginSingleTimeComputeCommand();
	}
	void D3D12Context::EndSingleTimeComputeCommand(Ref<D3D12CommandBuffer>& commandList)
	{
		m_CommandPool.EndSingleTimeComputeCommand(commandList);
	}
	Ref<D3D12CommandBuffer> D3D12Context::BeginSingleTimeTransferCommand()
	{
		return m_CommandPool.BeginSingleTimeTransferCommand();
	}
	void D3D12Context::EndSingleTimeTransferCommand(Ref<D3D12CommandBuffer>& commandList)
	{
		m_CommandPool.EndSingleTimeTransferCommand(commandList);
	}
	std::pair<Microsoft::WRL::ComPtr<ID3D12Fence>, uint64_t> D3D12Context::GetFence(CommandBufferType type)
	{
		std::pair<Microsoft::WRL::ComPtr<ID3D12Fence>, uint64_t> res;

		CheckAndResetFences();

		res.first = m_Fences.at(UINT(type) - 1);
		res.second = m_FenceValues.at(UINT(type) - 1);
		return res;
	}
	void D3D12Context::IncreaseFenceValue(CommandBufferType type)
	{
		CheckAndResetFences();

		m_FenceValues.at(UINT(type) - 1)++;
	}
	void D3D12Context::WaitForFence(CommandBufferType type, uint64_t timeout)
	{
		if(m_Fences.at(UINT(type) - 1)->GetCompletedValue() < m_FenceValues.at(UINT(type) - 1))
		{
			m_Fences.at(UINT(type) - 1)->SetEventOnCompletion(m_FenceValues.at(UINT(type) - 1), m_FenceEvents.at(UINT(type) - 1));
			WaitForSingleObject(m_FenceEvents.at(UINT(type) - 1), timeout);
		}
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
		if (IsD3D12DebugLayerRequested())
		{
			Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
			D3D12_THROW_IF_FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
			debugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
#endif
		D3D12_THROW_IF_FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_Factory)));
	}
	void D3D12Context::CreateDevice()
	{
		if (m_UseWarpDevice)
		{
			Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
			D3D12_THROW_IF_FAILED(m_Factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
			D3D12_THROW_IF_FAILED(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Device)));
		}
		else
		{
			Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
			GetHardwareAdapter(m_Factory.Get(), &hardwareAdapter);
			D3D12_THROW_IF_FAILED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Device)));
		}
	}
	void D3D12Context::SetupDebugInfoQueue()
	{
#ifdef GE_DEBUG
		
		D3D12_THROW_IF_FAILED(m_Device->QueryInterface(IID_PPV_ARGS(&m_DebugInfoQueue)));
		// The debug layer retains validation messages in this queue.  A busy frame
		// loop can produce messages faster than a developer consumes them, so leave
		// diagnostics enabled but bound their retained memory.
		D3D12_THROW_IF_FAILED(m_DebugInfoQueue->SetMessageCountLimit(1024));

		D3D12_MESSAGE_SEVERITY suppressedSeverities[] =
		{
			D3D12_MESSAGE_SEVERITY_INFO,
			D3D12_MESSAGE_SEVERITY_MESSAGE
		};
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumSeverities = _countof(suppressedSeverities);
		filter.DenyList.pSeverityList = suppressedSeverities;
		D3D12_THROW_IF_FAILED(m_DebugInfoQueue->AddStorageFilterEntries(&filter));
		StartD3D12DebugInfoQueueLogger(m_DebugInfoQueue);

		/*D3D12_MESSAGE_ID hide[] =
		{
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,
			D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE,
		};

		D3D12_INFO_QUEUE_FILTER filter = {};
		filter.DenyList.NumIDs	= _countof(hide);
		filter.DenyList.pIDList = hide;
		D3D12_THROW_IF_FAILED(m_DebugInfoQueue->AddStorageFilterEntries(&filter));*/
#endif
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
		m_SwapChainWidth				= width;
		m_SwapChainHeight				= height;

		DXGI_SWAP_CHAIN_DESC1			swapChainDesc = {};
		swapChainDesc.BufferCount		= Graphics::GetFrameCount();
		swapChainDesc.Width				= width;
		swapChainDesc.Height			= height;
		swapChainDesc.Format			= m_BackBufferFormat;
		swapChainDesc.BufferUsage		= DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect		= DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count	= m_Samples;
		swapChainDesc.Flags				= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC		fullscreenDesc{};
		fullscreenDesc.RefreshRate			= { 0, 0 };
		fullscreenDesc.ScanlineOrdering		= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		fullscreenDesc.Scaling				= DXGI_MODE_SCALING_UNSPECIFIED;
		fullscreenDesc.Windowed				= TRUE;

		Microsoft::WRL::ComPtr<IDXGISwapChain1>	swapChain;
		switch (Graphics::GetWindowManagerAPI())
		{
		case Config::CONFIG_WINDOW_MANAGER_API_GLFW:
			D3D12_THROW_IF_FAILED(m_Factory->CreateSwapChainForHwnd(m_GraphicsQueue.Get(), glfwGetWin32Window((GLFWwindow*)m_WindowHandle), &swapChainDesc, &fullscreenDesc, nullptr, &swapChain));
			break;
		case Config::CONFIG_WINDOW_MANAGER_API_WIN32:
			D3D12_THROW_IF_FAILED(m_Factory->CreateSwapChainForHwnd(m_GraphicsQueue.Get(), (HWND)m_WindowHandle, &swapChainDesc, &fullscreenDesc, nullptr, &swapChain));
			break;
		default:
			GE_CORE_ASSERT(false, "Unsupported Window Manager API!");
			break;
		}
		D3D12_THROW_IF_FAILED(swapChain.As(&m_SwapChain));
	}
	void D3D12Context::CreateCommandAllocator()
	{
		m_CommandPool = D3D12CommandPool(Graphics::GetCommandBufferCount());
	}
	void D3D12Context::CreateFrameResources()
	{
	}
	void D3D12Context::GetDescriptorSize()
	{
		m_RtvDescriptorIncrementSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_DsvDescriptorIncrementSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_CbvSrvUavDescriptorIncrementSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_SamplerDescriptorIncrementSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	}
	void D3D12Context::CreateRenderPass()
	{
		m_RenderPassSpec.BackBufferFormat				= {m_BackBufferFormat};
		m_RenderPassSpec.EnableDepthStencil				= true;
		m_RenderPassSpec.Samples						= m_Samples;
		m_RenderPassSpec.Operation.ColorBegin			= RENDER_PASS_BEGINE_OP_CLEAR;
		m_RenderPassSpec.Operation.ColorEnd				= RENDER_PASS_END_OP_STORE;
		m_RenderPassSpec.Operation.DepthStencilBegin	= RENDER_PASS_BEGINE_OP_CLEAR;
		m_RenderPassSpec.Operation.DepthStencilEnd		= RENDER_PASS_END_OP_STORE;

		m_SwapChainRenderPass					= CreateRef<D3D12RenderPass>(m_RenderPassSpec);
	}
	void D3D12Context::CreateRenderTargets()
	{
		m_RenderTargets.resize(Graphics::GetFrameCount());
		for (int i = 0; i < Graphics::GetFrameCount(); i++)
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> rt;
			D3D12_THROW_IF_FAILED(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(rt.GetAddressOf())));

			FrameBufferSpecificationForD3D12	spec{};
			spec.RenderTargets					= { rt };
			spec.Width							= m_SwapChainWidth;
			spec.Height							= m_SwapChainHeight;

			m_RenderTargets[i] = CreateRef<D3D12FrameBuffer>(m_SwapChainRenderPass, spec, m_RenderPassSpec);	
		}
	}
	void D3D12Context::CreateFences()
	{
		// each queue has its own fence, and each fence has its own value
		m_Fences.resize(m_QueueCount);
		m_FenceValues.resize(m_QueueCount, 0);
		m_FenceEvents.resize(m_QueueCount);
		for (int i = 0; i < m_QueueCount; i++)
		{
			m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fences[i]));
			m_FenceEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		}
	}
	void D3D12Context::CreateDescriptorHeaps()
	{
		m_HeapPool.Initialize(256, 256, 4096, 256);
	}
	void D3D12Context::CheckAndResetFences()
	{
		for (int i = 0; i < m_QueueCount; i++)
		{
			if (m_FenceValues.at(i) >= UINT64_MAX - 1024)
			{
				if (m_Fences.at(i)->GetCompletedValue() < m_FenceValues.at(i))
				{
					m_Fences.at(i)->SetEventOnCompletion(m_FenceValues.at(i), m_FenceEvents.at(i));
					WaitForSingleObject(m_FenceEvents.at(i), INFINITE);
				}
				m_FenceValues.at(i) = 0;
				// for convenience, we use the graphics queue to signal the fence, but it doesn't matter which queue we use to signal the fence, as long as we use the same queue to wait for the fence.
				m_GraphicsQueue->Signal(m_Fences.at(i).Get(), m_FenceValues.at(i));
			}
		}
	}
}
