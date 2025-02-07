#include "GEpch.h"
#include "RenderPass.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLRenderPass.h"
#include "Platform/Vulkan/VulkanRenderPass.h"

namespace GEngine
{
	std::vector<Ref<RenderPass>> RenderPass::s_RenderPasses;

	Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: {
			GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		}
		case RendererAPI::API::OpenGL: {
			Ref<RenderPass> pass = GetRenderPass(spec);
			if (pass == nullptr)
			{
				pass = CreateRef<OpenGLRenderPass>(spec);
			}
			return pass;
		}
		case RendererAPI::API::Vulkan: {
			Ref<RenderPass> pass = GetRenderPass(spec);
			if (pass == nullptr)
			{
				pass = CreateRef<VulkanRenderPass>(spec);
			}
			return pass;
		}
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<RenderPass> RenderPass::GetRenderPass(const RenderPassSpecification& spec)
	{
		for (auto pass : s_RenderPasses)
		{
			if (pass->GetSpecification() == spec)
			{
				return pass;
			}
		}
		return nullptr;
	}
}