#include "GEpch.h"
#include "D3D12ImGui.h"
#include "D3D12Utils.h"
#include "GEngine/Application.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/D3D12/D3D12Context.h"
#include "ImGui/backends/imgui_impl_glfw.h"
#include "ImGui/backends/imgui_impl_win32.h"
#include "ImGui/backends/imgui_impl_dx12.h"

namespace GEngine
{
	static ID3D12DescriptorHeap* s_RtvHeap = nullptr;
	static ID3D12DescriptorHeap* s_SrvHeap = nullptr;

	D3D12ImGui::~D3D12ImGui()
	{
		ImGui_ImplDX12_Shutdown();
	}
	void D3D12ImGui::OnAttach(void* window)
	{
		D3D12_DESCRIPTOR_HEAP_DESC	rtvHeapDesc = {};
		rtvHeapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.NumDescriptors	= Graphics::GetFrameCount();
		rtvHeapDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask		= 1;
		D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&s_RtvHeap)));


		D3D12_DESCRIPTOR_HEAP_DESC	srvHeapDesc = {};
		srvHeapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.NumDescriptors	= 1;
		srvHeapDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&s_SrvHeap)));
		

		switch (Application::Get().GetConfig()->GetWindowManagerAPI())
		{
		case Config::CONFIG_WINDOW_MANAGER_API_GLFW:
			ImGui_ImplGlfw_InitForOther((GLFWwindow*)window, true);
			break;
		case Config::CONFIG_WINDOW_MANAGER_API_WIN32:
			ImGui_ImplWin32_Init(window);
			break;
		default:
			GE_CORE_ASSERT(false, "Unknown window manager api");
			break;
		}

		ImGui_ImplDX12_Init(D3D12Context::Get()->GetDevice().Get(),
			Graphics::GetFrameCount(),
			DXGI_FORMAT_R8G8B8A8_UNORM, s_SrvHeap,
			s_SrvHeap->GetCPUDescriptorHandleForHeapStart(),
			s_SrvHeap->GetGPUDescriptorHandleForHeapStart());
	}
	void D3D12ImGui::Begin()
	{
		ImGui_ImplDX12_NewFrame();
	}
	void D3D12ImGui::End()
	{
	}
	Ref<Texture2D> D3D12ImGui::GetImGuiTexture()
	{
		return Ref<Texture2D>();
	}
	Ref<CommandBuffer> D3D12ImGui::GetCommandBuffer()
	{
		return Ref<CommandBuffer>();
	}
}