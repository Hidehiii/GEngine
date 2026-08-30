#include "GEpch.h"
#include "GEngine/Renderer/RenderSystem.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Graphics/GraphicsPresent.h"
#include "GEngine/ImGui/ImGuiLayer.h"
#include "GEngine/LayerStack.h"
#include "Platform/D3D12/D3D12CommandBuffer.h"

namespace
{
	D3D12_RESOURCE_STATES ToD3D12ResourceState(GEngine::RenderGraph::ResourceState state)
	{
		using ResourceState = GEngine::RenderGraph::ResourceState;
		switch (state)
		{
		case ResourceState::RenderTarget: return D3D12_RESOURCE_STATE_RENDER_TARGET;
		case ResourceState::DepthWrite: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
		case ResourceState::ShaderRead: return D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
		case ResourceState::ShaderWrite: return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		case ResourceState::CopySource: return D3D12_RESOURCE_STATE_COPY_SOURCE;
		case ResourceState::CopyDestination: return D3D12_RESOURCE_STATE_COPY_DEST;
		case ResourceState::Present: return D3D12_RESOURCE_STATE_PRESENT;
		case ResourceState::Undefined: return D3D12_RESOURCE_STATE_COMMON;
		}
		return D3D12_RESOURCE_STATE_COMMON;
	}
}

namespace GEngine
{
	RenderSystem::RenderSystem() = default;
	RenderSystem::~RenderSystem() = default;

	void RenderSystem::Configure(const GraphicsSpecification& specification)
	{
		GE_CORE_ASSERT(!m_Configured, "RenderSystem can only be configured once.");
		Graphics::Setup(specification);
		m_Configured = true;
	}

	void RenderSystem::Initialize()
	{
		GE_CORE_ASSERT(m_Configured, "RenderSystem must be configured before it initializes.");
		m_Present = GraphicsPresent::Create();
		m_RenderGraph.SetTransitionCallback([this](const FrameContext&, RenderGraph::ResourceHandle resource, RenderGraph::ResourceState before, RenderGraph::ResourceState after)
		{
			if (Graphics::GetGraphicsAPI() != GRAPHICS_API_DIRECT3DX12)
				return;

			auto* nativeResource = static_cast<ID3D12Resource*>(m_RenderGraph.GetNativeResource(resource));
			if (nativeResource == nullptr || before == after)
				return;

			auto commandBuffer = std::dynamic_pointer_cast<D3D12CommandBuffer>(GraphicsPresent::GetCommandBuffer());
			GE_CORE_ASSERT(commandBuffer, "D3D12 render-graph transitions require a D3D12 graphics command buffer.");
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(nativeResource, ToD3D12ResourceState(before), ToD3D12ResourceState(after));
			commandBuffer->GetCommandList()->ResourceBarrier(1, &barrier);
		});
		Graphics::Init();
	}

	void RenderSystem::Shutdown()
	{
		m_RenderGraph.Reset();
		m_Present.reset();
		m_Configured = false;
	}

	void RenderSystem::RenderFrame(LayerStack& layers, ImGuiLayer* imGuiLayer)
	{
		GE_CORE_ASSERT(!m_FrameOpen, "The previous render frame has not finished.");
		m_FrameContext.Begin(m_FrameNumber, Graphics::GetFrame());
		if (!m_Present || !m_Present->AcquireFrame(m_FrameContext))
			return;
		Graphics::SelectFrame(m_FrameContext.GetFrameIndex());
		m_FrameOpen = true;
		m_Present->BeginFrame(m_FrameContext);

		m_RenderGraph.Reset();
		const auto renderPass = m_RenderGraph.AddPass("LayerRender", [&layers](FrameContext&)
		{
			for (auto* layer : layers)
				layer->OnRender();
		});
		auto lastPass = renderPass;
		if (imGuiLayer)
		{
			const auto uiPass = m_RenderGraph.AddPass("ImGui", [&layers, imGuiLayer](FrameContext&)
			{
				imGuiLayer->Begin();
				for (auto* layer : layers)
					layer->OnImGuiRender();
				imGuiLayer->End();
			});
			m_RenderGraph.AddDependency(uiPass, renderPass);
			lastPass = uiPass;
		}
		const auto presentPass = m_RenderGraph.AddPass("Present", [this, &layers](FrameContext&)
		{
			m_Present->Begin();
			for (auto* layer : layers)
				layer->OnPresent();
		});

		m_RenderGraph.AddDependency(presentPass, lastPass);

		if (void* nativeResource = m_Present->GetPresentationNativeResource())
		{
			const auto backBuffer = m_RenderGraph.ImportExternalResource("PresentationBackBuffer", nativeResource, RenderGraph::ResourceState::Present);
			m_RenderGraph.Write(presentPass, backBuffer, RenderGraph::ResourceState::RenderTarget);
			const auto releasePass = m_RenderGraph.AddPass("ReleasePresentation", []() {});
			m_RenderGraph.Write(releasePass, backBuffer, RenderGraph::ResourceState::Present);
		}
		m_RenderGraph.Execute(m_FrameContext);
		m_Present->EndFrame(m_FrameContext);
	}

	void RenderSystem::Resize(uint32_t width, uint32_t height)
	{
		Graphics::SetViewport(width, height);
	}

	void RenderSystem::AdvanceFrame()
	{
		GE_CORE_ASSERT(!m_FrameOpen || m_FrameContext.IsSubmitted(), "A render frame must be submitted before it advances.");
		Graphics::FrameMove();
		++m_FrameNumber;
		m_FrameOpen = false;
	}
}
