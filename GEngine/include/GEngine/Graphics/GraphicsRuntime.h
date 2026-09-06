#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsAPI.h"

namespace GEngine
{
	class CommandBuffer;

	// Owns the device-facing state for one engine runtime.  The legacy Graphics
	// facade forwards to the active instance while callers are migrated to
	// explicit device ownership.
	class GENGINE_API GraphicsRuntime
	{
	public:
		explicit GraphicsRuntime(const GraphicsSpecification& specification);
		~GraphicsRuntime();

		void Initialize();
		void Shutdown();
		void AdvanceFrame();
		void SelectFrame(uint8_t frameIndex);
		void SetViewport(uint32_t width, uint32_t height);
		void SetReverseDepth(bool reverse);
		void SetCommandsBarrier(Ref<CommandBuffer>& first, Ref<CommandBuffer>& second);
		void SubmitCommandBuffer(const Ref<CommandBuffer>& commandBuffer);
		void TransitionResource(const Ref<CommandBuffer>& commandBuffer, const Ref<GraphicsResource>& resource,
			GraphicsResourceState before, GraphicsResourceState after);

		Graphics_API GetGraphicsAPI() const;
		uint8_t GetFrameCount() const { return m_FrameCount; }
		uint8_t GetFrame() const { return m_Frame; }
		uint32_t GetCommandBufferCount() const { return m_CommandBufferCount; }
		uint8_t GetWindowManagerAPI() const { return m_WindowManagerAPI; }
		bool IsReverseDepth() const { return m_ReverseDepth; }
		GraphicsCapabilities GetCapabilities() const;
		RenderDevice& GetRenderDevice();
		Ref<CommandBuffer> GetGraphicsCommandBuffer();
		Ref<CommandBuffer> GetComputeCommandBuffer();
		uint32_t GetMaxTexture2DSize();
		uint32_t GetMaxCombinedTextureCount();
		uint32_t GetMaxPerStageTextureCount();
		uint32_t GetMaxTextureArrayLayers();
		uint32_t GetMinUniformBufferOffsetAlignment();
		uint32_t GetMaxUniformBufferSize();
		Vector3 GetMaxComputeWorkGroupCount();
		Vector3 GetMaxComputeWorkGroupSize();
		uint32_t GetMaxComputeWorkGroupInvocations();
		uint32_t GetViewportWidth() const { return m_ViewportWidth; }
		uint32_t GetViewportHeight() const { return m_ViewportHeight; }

	private:
		GraphicsAPI& RequireDevice() const;

	private:
		Scope<GraphicsAPI> m_GraphicsAPI;
		uint8_t m_FrameCount = 0;
		uint8_t m_Frame = 0;
		uint32_t m_CommandBufferCount = 0;
		uint8_t m_WindowManagerAPI = 0;
		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;
		bool m_ReverseDepth = false;
		bool m_Initialized = false;
	};
}
