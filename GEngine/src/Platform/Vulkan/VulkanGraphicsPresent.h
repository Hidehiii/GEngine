#pragma once
#include "GEngine/Renderer/GraphicsPresent.h"


namespace GEngine
{
	class GENGINE_API VulkanGraphicsPresent : public GraphicsPresent
	{
	public:
		VulkanGraphicsPresent() {}
		virtual void Begin() override;
		virtual void End() override;

	private:
		uint32_t	m_SwapChainImageIndex = 0;
		bool		m_RecreateSwapChain = false;
	};
}


