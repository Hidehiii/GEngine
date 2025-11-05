#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsCommon.h"

namespace GEngine
{
	

	



	class GENGINE_API RenderPass
	{
	public:
		virtual ~RenderPass() = default;

		virtual RenderPassSpecification GetSpecification() { return m_Specification; }

		static Ref<RenderPass> GetRenderPass(const RenderPassSpecification& spec);

		static Ref<RenderPass> Create(const RenderPassSpecification& spec);
		
	protected:
		RenderPassSpecification		m_Specification;
	protected:
		static std::vector<Ref<RenderPass>>		s_RenderPasses;
	};
}


