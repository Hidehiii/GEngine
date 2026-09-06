#include "GEpch.h"
#include "GEngine/Graphics/GraphicsRuntime.h"

#include "GEngine/Graphics/RenderPass.h"
#include "GEngine/Graphics/Sampler.h"
#include "GEngine/Graphics/Shader.h"
#include "GEngine/Graphics/Texture.h"
#include "GEngine/Tools/ShaderCompiler.h"
#include "Platform/D3D12/D3D12GraphicsAPI.h"
#include "Platform/OpenGL/OpenGLGraphicsAPI.h"
#include "Platform/Vulkan/VulkanGraphicsAPI.h"

namespace GEngine
{
	GraphicsRuntime::GraphicsRuntime(const GraphicsSpecification& specification)
		: m_FrameCount(specification.FramesInFlight),
		m_CommandBufferCount(specification.CommandBufferCount),
		m_WindowManagerAPI(specification.WindowManagerAPI),
		m_ViewportWidth(specification.ViewportWidth),
		m_ViewportHeight(specification.ViewportHeight)
	{
		switch (specification.API)
		{
		case GRAPHICS_API_OPENGL:
			m_GraphicsAPI = CreateScope<OpenGLGraphicsAPI>();
			break;
		case GRAPHICS_API_VULKAN:
			m_GraphicsAPI = CreateScope<VulkanGraphicsAPI>();
			break;
		case GRAPHICS_API_DIRECT3DX12:
			m_GraphicsAPI = CreateScope<D3D12GraphicsAPI>();
			break;
		case GRAPHICS_API_NONE:
		default:
			GE_CORE_ASSERT(false, "Renderer API is not supported.");
			break;
		}
	}

	GraphicsRuntime::~GraphicsRuntime()
	{
		Shutdown();
	}

	void GraphicsRuntime::Initialize()
	{
		if (m_Initialized)
			return;
		ShaderCompiler::Create();
		m_Initialized = true;
	}

	void GraphicsRuntime::Shutdown()
	{
		if (!m_GraphicsAPI)
			return;

		// Shared render objects own backend resources.  The RenderSystem destroys
		// this runtime before its Window, so the device/context remains valid here.
		RenderPass::ShutdownCache();
		Shader::ShutdownCache();
		Sampler::ShutdownCache();
		CubeMap::ShutdownCache();
		Texture2D::ShutdownCache();

		m_GraphicsAPI.reset();
		m_FrameCount = 0;
		m_Frame = 0;
		m_CommandBufferCount = 0;
		m_ViewportWidth = 0;
		m_ViewportHeight = 0;
		m_ReverseDepth = false;
		m_Initialized = false;
	}

	void GraphicsRuntime::AdvanceFrame()
	{
		GE_CORE_ASSERT(m_FrameCount > 0, "Graphics runtime has no frame slots.");
		m_Frame = m_Frame + 1 < m_FrameCount ? m_Frame + 1 : 0;
	}

	void GraphicsRuntime::SelectFrame(uint8_t frameIndex)
	{
		GE_CORE_ASSERT(frameIndex < m_FrameCount, "The selected frame slot is outside the configured frame count.");
		m_Frame = frameIndex;
	}

	void GraphicsRuntime::SetViewport(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;
	}

	void GraphicsRuntime::SetReverseDepth(bool reverse)
	{
		m_ReverseDepth = reverse;
	}

	void GraphicsRuntime::SetCommandsBarrier(Ref<CommandBuffer>& first, Ref<CommandBuffer>& second)
	{
		RequireDevice().SetCommandsBarrier(first, second);
	}

	void GraphicsRuntime::SubmitCommandBuffer(const Ref<CommandBuffer>& commandBuffer)
	{
		RequireDevice().SubmitCommandBuffer(commandBuffer);
	}

	void GraphicsRuntime::TransitionResource(const Ref<CommandBuffer>& commandBuffer, const Ref<GraphicsResource>& resource,
		GraphicsResourceState before, GraphicsResourceState after)
	{
		RequireDevice().TransitionResource(commandBuffer, resource, before, after);
	}

	Graphics_API GraphicsRuntime::GetGraphicsAPI() const { return RequireDevice().GetAPI(); }
	GraphicsCapabilities GraphicsRuntime::GetCapabilities() const { return RequireDevice().GetCapabilities(); }
	RenderDevice& GraphicsRuntime::GetRenderDevice() { return RequireDevice(); }
	Ref<CommandBuffer> GraphicsRuntime::GetGraphicsCommandBuffer() { return RequireDevice().GetGraphicsCommandBuffer(); }
	Ref<CommandBuffer> GraphicsRuntime::GetComputeCommandBuffer() { return RequireDevice().GetComputeCommandBuffer(); }
	uint32_t GraphicsRuntime::GetMaxTexture2DSize() { return RequireDevice().GetMaxTexture2DSize(); }
	uint32_t GraphicsRuntime::GetMaxCombinedTextureCount() { return RequireDevice().GetMaxCombinedTextureCount(); }
	uint32_t GraphicsRuntime::GetMaxPerStageTextureCount() { return RequireDevice().GetMaxPerStageTextureCount(); }
	uint32_t GraphicsRuntime::GetMaxTextureArrayLayers() { return RequireDevice().GetMaxTextureArrayLayers(); }
	uint32_t GraphicsRuntime::GetMinUniformBufferOffsetAlignment() { return RequireDevice().GetMinUniformBufferOffsetAlignment(); }
	uint32_t GraphicsRuntime::GetMaxUniformBufferSize() { return RequireDevice().GetMaxUniformBufferSize(); }
	Vector3 GraphicsRuntime::GetMaxComputeWorkGroupCount() { return RequireDevice().GetMaxComputeWorkGroupCount(); }
	Vector3 GraphicsRuntime::GetMaxComputeWorkGroupSize() { return RequireDevice().GetMaxComputeWorkGroupSize(); }
	uint32_t GraphicsRuntime::GetMaxComputeWorkGroupInvocations() { return RequireDevice().GetMaxComputeWorkGroupInvocations(); }

	GraphicsAPI& GraphicsRuntime::RequireDevice() const
	{
		GE_CORE_ASSERT(m_GraphicsAPI != nullptr, "Graphics runtime has been shut down.");
		return *m_GraphicsAPI;
	}
}
