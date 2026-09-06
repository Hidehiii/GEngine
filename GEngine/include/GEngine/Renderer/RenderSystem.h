#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsAPI.h"
#include "GEngine/Renderer/FrameContext.h"
#include "GEngine/Renderer/RenderGraph.h"

namespace GEngine
{
	class GraphicsPresent;
	class GraphicsRuntime;
	class LayerStack;
	class ImGuiLayer;

	class GENGINE_API RenderSystem
	{
	public:
		RenderSystem();
		~RenderSystem();

		void Configure(const GraphicsSpecification& specification);
		void Initialize();
		void Shutdown();
		void RenderFrame(LayerStack& layers, ImGuiLayer* imGuiLayer);
		void Resize(uint32_t width, uint32_t height);
		void AdvanceFrame();
		void WaitForIdle();
		const FrameContext& GetCurrentFrameContext() const { return m_FrameContext; }

	private:
		Scope<GraphicsRuntime> m_GraphicsRuntime;
		Scope<GraphicsPresent> m_Present;
		RenderGraph m_RenderGraph;
		FrameContext m_FrameContext;
		uint64_t m_FrameNumber = 0;
		bool m_FrameOpen = false;
		bool m_Configured = false;
	};
}
