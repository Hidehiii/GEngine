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
		virtual const std::vector<RenderPassSpecification> GetSubpassSpecifications() const { return m_SubpassSpecifications; }

		static Ref<RenderPass> GetRenderPass(const RenderPassSpecification& spec);

		static Ref<RenderPass> Create(const RenderPassSpecification& spec);
		static Ref<RenderPass> Create(const RenderPassSpecification& spec, const std::initializer_list<RenderPassSpecification>& subpassSpecs);
	protected:
		RenderPassSpecification					m_Specification;
		std::vector<RenderPassSpecification>	m_SubpassSpecifications;
	protected:
		static std::vector<Ref<RenderPass>>		s_RenderPasses;
	};
}


