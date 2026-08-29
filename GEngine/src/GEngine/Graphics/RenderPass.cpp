#include "GEpch.h"
#include "RenderPass.h"
#include "Graphics.h"

namespace GEngine
{
	std::vector<Ref<RenderPass>> RenderPass::s_RenderPasses;

	Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& spec)
	{
		Ref<RenderPass> pass = GetRenderPass(spec);
		if (pass == nullptr)
		{
			pass = Graphics::GetRenderDevice().CreateRenderPass(spec);
			s_RenderPasses.push_back(pass);
		}
		return pass;
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
