#include "GEpch.h"
#include "RenderPass.h"
#include "Graphics.h"
#include "Platform/OpenGL/OpenGLRenderPass.h"
#include "Platform/Vulkan/VulkanRenderPass.h"

namespace GEngine
{
	std::vector<Ref<RenderPass>> RenderPass::s_RenderPasses;

	Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& spec)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GraphicsAPI::API::None: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GraphicsAPI::API::OpenGL: {
			Ref<RenderPass> pass = GetRenderPass(spec);
			if (pass == nullptr)
			{
				pass = CreateRef<OpenGLRenderPass>(spec);
				s_RenderPasses.push_back(pass);
			}
			return pass;
		}
		case GraphicsAPI::API::Vulkan: {
			Ref<RenderPass> pass = GetRenderPass(spec);
			if (pass == nullptr)
			{
				pass = CreateRef<VulkanRenderPass>(spec);
				s_RenderPasses.push_back(pass);
			}
			return pass;
		}
		}

		GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
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