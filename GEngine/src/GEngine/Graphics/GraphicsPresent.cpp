#include "GEpch.h"
#include "GEngine/Graphics/GraphicsPresent.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/OpenGL/OpenGLGraphicsPresent.h"
#include "Platform/Vulkan/VulkanGraphicsPresent.h"
#include "Platform/D3D12/D3D12GraphicsPresent.h"
#include "GEngine/Graphics/CommandBuffer.h"

namespace GEngine
{
	GraphicsPresent*	GraphicsPresent::s_ActivePresenter = nullptr;

	GraphicsPresent::GraphicsPresent() = default;
	GraphicsPresent::~GraphicsPresent()
	{
		if (s_ActivePresenter == this)
			s_ActivePresenter = nullptr;
	}

	void GraphicsPresent::SetActivePresenter(GraphicsPresent* presenter)
	{
		GE_CORE_ASSERT(presenter != nullptr, "An active graphics presenter is required.");
		s_ActivePresenter = presenter;
	}

	Ref<CommandBuffer> GraphicsPresent::GetCommandBuffer()
	{
		GE_CORE_ASSERT(s_ActivePresenter != nullptr, "No active graphics presenter is available.");
		return s_ActivePresenter->m_CommandBuffers.at(Graphics::GetFrame());
	}

	bool GraphicsPresent::AcquireFrame(FrameContext& frameContext)
	{
		if (!AquireImage())
			return false;

		frameContext.MarkAcquired(Graphics::GetFrame());
		return true;
	}

	void GraphicsPresent::BeginFrame(const FrameContext& frameContext)
	{
		GE_CORE_ASSERT(frameContext.IsAcquired(), "A presentation frame must be acquired before it begins.");
	}

	void GraphicsPresent::EndFrame(FrameContext& frameContext)
	{
		End();
		frameContext.MarkSubmitted(Submit());
	}

	void GraphicsPresent::Render(Ref<GraphicsPipeline>& pipeline, const int& pass, uint32_t instanceCount, uint32_t indexCount)
	{
		GetCommandBuffer()->Render(pipeline, pass, instanceCount, indexCount);
	}

	void GraphicsPresent::Compute(Ref<ComputePipeline>& pipeline, const int& pass, uint32_t x, uint32_t y, uint32_t z)
	{
		GetCommandBuffer()->Compute(pipeline, pass, x, y, z);
	}

	Scope<GraphicsPresent> GraphicsPresent::Create()
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_NONE:			GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case GRAPHICS_API_OPENGL:		return CreateScope<OpenGLGraphicsPresent>();
		case GRAPHICS_API_VULKAN:		return CreateScope<VulkanGraphicsPresent>();
		case GRAPHICS_API_DIRECT3DX12:	return CreateScope<D3D12GraphicsPresent>();
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
