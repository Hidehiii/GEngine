#include "GEpch.h"
#include "GEngine/Renderer/RenderSystem.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Graphics/GraphicsPresent.h"
#include "GEngine/Graphics/GraphicsRuntime.h"
#include "GEngine/ImGui/ImGuiLayer.h"
#include "GEngine/LayerStack.h"

namespace GEngine
{
	RenderSystem::RenderSystem() = default;
	RenderSystem::~RenderSystem()
	{
		Shutdown();
	}

	void RenderSystem::Configure(const GraphicsSpecification& specification)
	{
		GE_CORE_ASSERT(!m_Configured, "RenderSystem can only be configured once.");
		m_GraphicsRuntime = CreateScope<GraphicsRuntime>(specification);
		Graphics::SetActiveRuntime(m_GraphicsRuntime.get());
		m_Configured = true;
	}

	void RenderSystem::Initialize()
	{
		GE_CORE_ASSERT(m_Configured, "RenderSystem must be configured before it initializes.");
		m_Present = GraphicsPresent::Create();
		GraphicsPresent::SetActivePresenter(m_Present.get());
		m_RenderGraph.SetTransitionCallback([](const FrameContext&, const Ref<GraphicsResource>& resource, RenderGraph::ResourceState before, RenderGraph::ResourceState after)
		{
			if (!resource || before == after || !resource->RequiresExplicitStateTransition())
				return;
			Graphics::TransitionResource(GraphicsPresent::GetCommandBuffer(), resource, before, after);
		});
		m_GraphicsRuntime->Initialize();
	}

	void RenderSystem::Shutdown()
	{
		m_RenderGraph.Reset();
		m_Present.reset();
		if (m_GraphicsRuntime)
		{
			m_GraphicsRuntime->Shutdown();
			Graphics::SetActiveRuntime(nullptr);
			m_GraphicsRuntime.reset();
		}
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

		if (auto presentationResource = m_Present->GetPresentationResource())
		{
			const auto backBuffer = m_RenderGraph.ImportExternalResource("PresentationBackBuffer", presentationResource,
				RenderGraph::ResourceState::Present);
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
