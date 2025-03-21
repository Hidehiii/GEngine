#include "GEpch.h"
#include "GraphicsPresent.h"
#include "GEngine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLGraphicsPresent.h"
#include "Platform/Vulkan/VulkanGraphicsPresent.h"
#include "GEngine/Renderer/CommandBuffer.h"

namespace GEngine
{
	std::vector<Ref<CommandBuffer>>	GraphicsPresent::s_CommandBuffers;

	Ref<CommandBuffer> GraphicsPresent::GetCommandBuffer()
	{
		return s_CommandBuffers.at(Renderer::GetCurrentFrame());
	}

	void GraphicsPresent::Render(Ref<Scene>& scene)
	{
		GetCommandBuffer()->Render(scene);
	}

	void GraphicsPresent::Render(Ref<GraphicsPipeline>& pipeline, uint32_t instanceCount, uint32_t indexCount)
	{
		GetCommandBuffer()->Render(pipeline, instanceCount, indexCount);
	}

	void GraphicsPresent::Compute(Ref<ComputePipeline>& pipeline, uint32_t x, uint32_t y, uint32_t z)
	{
		GetCommandBuffer()->Compute(pipeline, x, y, z);
	}

	Scope<GraphicsPresent> GraphicsPresent::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return CreateScope<OpenGLGraphicsPresent>();
		case RendererAPI::API::Vulkan:  return CreateScope<VulkanGraphicsPresent>();
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}