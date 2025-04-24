#include "GEpch.h"
#include "GraphicsPresent.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/OpenGL/OpenGLGraphicsPresent.h"
#include "Platform/Vulkan/VulkanGraphicsPresent.h"
#include "GEngine/Graphics/CommandBuffer.h"

namespace GEngine
{
	std::vector<Ref<CommandBuffer>>	GraphicsPresent::s_CommandBuffers;

	Ref<CommandBuffer> GraphicsPresent::GetCommandBuffer()
	{
		return s_CommandBuffers.at(Graphics::GetFrame());
	}

	void GraphicsPresent::Render(Ref<GraphicsPipeline>& pipeline, const std::string& pass, uint32_t instanceCount, uint32_t indexCount)
	{
		GetCommandBuffer()->Render(pipeline, pass, instanceCount, indexCount);
	}

	void GraphicsPresent::Compute(Ref<ComputePipeline>& pipeline, const std::string& pass, uint32_t x, uint32_t y, uint32_t z)
	{
		GetCommandBuffer()->Compute(pipeline, pass, x, y, z);
	}

	Scope<GraphicsPresent> GraphicsPresent::Create()
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GraphicsAPI::API::None:    GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case GraphicsAPI::API::OpenGL:  return CreateScope<OpenGLGraphicsPresent>();
		case GraphicsAPI::API::Vulkan:  return CreateScope<VulkanGraphicsPresent>();
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}