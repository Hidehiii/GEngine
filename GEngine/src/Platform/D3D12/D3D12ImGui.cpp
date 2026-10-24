#include "GEpch.h"
#include "Platform/D3D12/D3D12ImGui.h"
#include "Platform/D3D12/D3D12Utils.h"
#include "GEngine/Application.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/D3D12/D3D12Context.h"
#include "Platform/D3D12/D3D12FrameBuffer.h"
#include "Platform/D3D12/D3D12Texture2D.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx12.h"

namespace GEngine
{
	static RenderPassSpecification				s_Spec;
	static Ref<D3D12RenderPass>					s_RenderPass = nullptr;
	static Ref<D3D12FrameBuffer>				s_RenderTarget = nullptr;
	static ID3D12DescriptorHeap*				s_SrvHeap = nullptr;
	static std::vector<UINT> s_FreeDescriptors;
	static std::vector<std::vector<std::pair<UINT, Ref<Texture2D>>>> s_FrameTextures;
	static std::vector<Ref<D3D12CommandBuffer>>	s_CommandBuffers;

	D3D12ImGui::~D3D12ImGui()
	{
		ImGui_ImplDX12_Shutdown();
		s_FrameTextures.clear();
		s_CommandBuffers.clear();
		s_RenderTarget.reset();
		s_RenderPass.reset();
		if (s_SrvHeap)
			s_SrvHeap->Release();
		s_SrvHeap = nullptr;
		s_FreeDescriptors.clear();
	}
	void D3D12ImGui::OnAttach(void* window)
	{
		CreateCommandBuffers();
		s_FrameTextures.resize(Graphics::GetFrameCount());
		s_Spec.RenderTargets				= { FRAME_BUFFER_TEXTURE_FORMAT_RGBA8 };
		s_Spec.DepthStencil					= FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8;
		s_Spec.Samples						= 1;
		s_Spec.Operation.ColorBegin			= RENDER_PASS_BEGINE_OP_CLEAR;
		s_Spec.Operation.ColorEnd			= RENDER_PASS_END_OP_STORE;
		s_Spec.Operation.DepthStencilBegin	= RENDER_PASS_BEGINE_OP_CLEAR;
		s_Spec.Operation.DepthStencilEnd	= RENDER_PASS_END_OP_STORE;

		s_RenderPass = CreateRef<D3D12RenderPass>(s_Spec);
		GE_CORE_ASSERT(s_RenderPass != nullptr, "Failed to create D3D12RenderPass for ImGui");

		s_RenderTarget = CreateRef<D3D12FrameBuffer>(s_RenderPass, Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
		GE_CORE_ASSERT(s_RenderTarget != nullptr, "Failed to create D3D12FrameBuffer for ImGui");

		D3D12_DESCRIPTOR_HEAP_DESC	srvHeapDesc = {};
		srvHeapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.NumDescriptors	= 10000;
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

		for (UINT i = srvHeapDesc.NumDescriptors; i > 0; --i)
			s_FreeDescriptors.push_back(i - 1);
		ImGui_ImplDX12_InitInfo info{};
		info.Device = D3D12Context::Get()->GetDevice().Get();
		info.CommandQueue = D3D12Context::Get()->GetGraphicsQueue().Get();
		info.NumFramesInFlight = Graphics::GetFrameCount();
		info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		info.SrvDescriptorHeap = s_SrvHeap;
		info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info,
			D3D12_CPU_DESCRIPTOR_HANDLE* cpu, D3D12_GPU_DESCRIPTOR_HANDLE* gpu)
		{
			if (s_FreeDescriptors.empty())
				throw std::runtime_error("ImGui SRV descriptor heap exhausted");
			const UINT index = s_FreeDescriptors.back();
			s_FreeDescriptors.pop_back();
			const auto stride = info->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			*cpu = s_SrvHeap->GetCPUDescriptorHandleForHeapStart();
			*gpu = s_SrvHeap->GetGPUDescriptorHandleForHeapStart();
			cpu->ptr += static_cast<SIZE_T>(index) * stride;
			gpu->ptr += static_cast<UINT64>(index) * stride;
		};
		info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* info,
			D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE)
		{
			const auto stride = info->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			s_FreeDescriptors.push_back(static_cast<UINT>((cpu.ptr - s_SrvHeap->GetCPUDescriptorHandleForHeapStart().ptr) / stride));
		};
		if (!ImGui_ImplDX12_Init(&info))
			throw std::runtime_error("Failed to initialize ImGui D3D12 backend");
	}
	void D3D12ImGui::Begin()
	{
		auto& textures = s_FrameTextures.at(Graphics::GetFrame());
		for (auto& entry : textures) s_FreeDescriptors.push_back(entry.first);
		textures.clear();
		if(s_RenderTarget->GetWidth() != Application::Get().GetWindow().GetWidth() ||
			s_RenderTarget->GetHeight() != Application::Get().GetWindow().GetHeight())
		{
			s_RenderTarget = CreateRef<D3D12FrameBuffer>(s_RenderPass, Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
		}
		ImGui_ImplDX12_NewFrame();
	}
	void D3D12ImGui::End()
	{
		auto cmdList = s_CommandBuffers.at(Graphics::GetFrame())->GetCommandList();
		auto cmdAllocator = s_CommandBuffers.at(Graphics::GetFrame())->GetCommandAllocator();

		// The presentation acquire path waited for this frame slot's previous
		// submission, so this allocator is safe to recycle before UI recording.
		D3D12_THROW_IF_FAILED(cmdAllocator->Reset());
		D3D12_THROW_IF_FAILED(cmdList->Reset(cmdAllocator.Get(), nullptr));
		s_RenderTarget->Begin(s_CommandBuffers.at(Graphics::GetFrame()).get());

		cmdList->SetDescriptorHeaps(1, &s_SrvHeap);
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList.Get());

		s_RenderTarget->End(s_CommandBuffers.at(Graphics::GetFrame()).get());
		D3D12_THROW_IF_FAILED(cmdList->Close());
		Graphics::SubmitCommandBuffer(s_CommandBuffers.at(Graphics::GetFrame()));
	}
	uint64_t D3D12ImGui::RegisterTexture(const Ref<Texture2D>& texture)
	{
		auto& textures = s_FrameTextures.at(Graphics::GetFrame());
		UINT index = UINT_MAX;
		for (auto& entry : textures)
			if (entry.second == texture) { index = entry.first; break; }
		if (index == UINT_MAX)
		{
			if (s_FreeDescriptors.empty()) throw std::runtime_error("ImGui texture descriptor heap exhausted");
			index = s_FreeDescriptors.back();
			s_FreeDescriptors.pop_back();
			auto native = std::dynamic_pointer_cast<D3D12Texture2D>(texture);
			if (!native) throw std::invalid_argument("ImGui requires a D3D12 texture");
			auto device = D3D12Context::Get()->GetDevice();
			auto cpu = s_SrvHeap->GetCPUDescriptorHandleForHeapStart();
			cpu.ptr += static_cast<SIZE_T>(index) * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			auto description = native->GetShaderResourceViewDesc();
			device->CreateShaderResourceView(native->GetResource().Get(), &description, cpu);
			textures.emplace_back(index, texture);
		}
		auto gpu = s_SrvHeap->GetGPUDescriptorHandleForHeapStart();
		gpu.ptr += static_cast<UINT64>(index) * D3D12Context::Get()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		return gpu.ptr;
	}

	Ref<Texture2D> D3D12ImGui::GetImGuiTexture()
	{
		return s_RenderTarget->GetRenderTarget(0);
	}
	Ref<CommandBuffer> D3D12ImGui::GetCommandBuffer()
	{
		return s_CommandBuffers.at(Graphics::GetFrame());
	}

	void D3D12ImGui::CreateCommandBuffers()
	{
		s_CommandBuffers.resize(Graphics::GetFrameCount());
		for (size_t i = 0; i < Graphics::GetFrameCount(); i++)
		{
			s_CommandBuffers[i] = CreateRef<D3D12CommandBuffer>(COMMAND_BUFFER_TYPE_GRAPHICS);
		}
	}
}
