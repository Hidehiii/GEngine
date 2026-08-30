#pragma once
#include "GEngine/Graphics/RenderPass.h"

namespace GEngine
{
	class GENGINE_API OpenGLRenderPass : public RenderPass
	{
	public:
		OpenGLRenderPass(const RenderPassSpecification& spec);
		virtual ~OpenGLRenderPass() override;
	};

}

