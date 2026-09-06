#include "GEpch.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Graphics/GraphicsRuntime.h"

namespace GEngine
{
	GraphicsRuntime* Graphics::s_ActiveRuntime = nullptr;

	void Graphics::FrameMove()
	{
		ActiveRuntime().AdvanceFrame();
	}

	void Graphics::SelectFrame(uint8_t frameIndex)
	{
		ActiveRuntime().SelectFrame(frameIndex);
	}

	void Graphics::SetViewport(uint32_t width, uint32_t height)
	{
		ActiveRuntime().SetViewport(width, height);
	}

	void Graphics::SetActiveRuntime(GraphicsRuntime* runtime)
	{
		s_ActiveRuntime = runtime;
	}

	GraphicsRuntime& Graphics::ActiveRuntime()
	{
		GE_CORE_ASSERT(s_ActiveRuntime != nullptr, "No active graphics runtime is available.");
		return *s_ActiveRuntime;
	}

	void Graphics::SetCommandsBarrier(Ref<CommandBuffer>& first, Ref<CommandBuffer>& second)
	{
		ActiveRuntime().SetCommandsBarrier(first, second);
	}

	void Graphics::SubmitCommandBuffer(const Ref<CommandBuffer>& commandBuffer)
	{
		ActiveRuntime().SubmitCommandBuffer(commandBuffer);
	}

	void Graphics::TransitionResource(const Ref<CommandBuffer>& commandBuffer, const Ref<GraphicsResource>& resource,
		GraphicsResourceState before, GraphicsResourceState after)
	{
		ActiveRuntime().TransitionResource(commandBuffer, resource, before, after);
	}

	void Graphics::SetReverseDepth(bool reverse)
	{
		ActiveRuntime().SetReverseDepth(reverse);
	}

	Graphics_API Graphics::GetGraphicsAPI() { return ActiveRuntime().GetGraphicsAPI(); }
	uint8_t Graphics::GetFrameCount() { return ActiveRuntime().GetFrameCount(); }
	uint8_t Graphics::GetFrame() { return ActiveRuntime().GetFrame(); }
	uint32_t Graphics::GetCommandBufferCount() { return ActiveRuntime().GetCommandBufferCount(); }
	uint8_t Graphics::GetWindowManagerAPI() { return ActiveRuntime().GetWindowManagerAPI(); }
	bool Graphics::IsReverseDepth() { return ActiveRuntime().IsReverseDepth(); }
	GraphicsCapabilities Graphics::GetCapabilities() { return ActiveRuntime().GetCapabilities(); }
	RenderDevice& Graphics::GetRenderDevice() { return ActiveRuntime().GetRenderDevice(); }
	Ref<CommandBuffer> Graphics::GetGraphicsCommandBuffer() { return ActiveRuntime().GetGraphicsCommandBuffer(); }
	Ref<CommandBuffer> Graphics::GetComputeCommandBuffer() { return ActiveRuntime().GetComputeCommandBuffer(); }
	uint32_t Graphics::GetMaxTexture2DSize() { return ActiveRuntime().GetMaxTexture2DSize(); }
	uint32_t Graphics::GetMaxCombinedTextureCount() { return ActiveRuntime().GetMaxCombinedTextureCount(); }
	uint32_t Graphics::GetMaxPerStageTextureCount() { return ActiveRuntime().GetMaxPerStageTextureCount(); }
	uint32_t Graphics::GetMaxTextureArrayLayers() { return ActiveRuntime().GetMaxTextureArrayLayers(); }
	uint32_t Graphics::GetMinUniformBufferOffsetAlignment() { return ActiveRuntime().GetMinUniformBufferOffsetAlignment(); }
	uint32_t Graphics::GetMaxUniformBufferSize() { return ActiveRuntime().GetMaxUniformBufferSize(); }
	Vector3 Graphics::GetMaxComputeWorkGroupCount() { return ActiveRuntime().GetMaxComputeWorkGroupCount(); }
	Vector3 Graphics::GetMaxComputeWorkGroupSize() { return ActiveRuntime().GetMaxComputeWorkGroupSize(); }
	uint32_t Graphics::GetMaxComputeWorkGroupInvocations() { return ActiveRuntime().GetMaxComputeWorkGroupInvocations(); }
	uint32_t Graphics::GetViewportWidth() { return ActiveRuntime().GetViewportWidth(); }
	uint32_t Graphics::GetViewportHeight() { return ActiveRuntime().GetViewportHeight(); }
}
