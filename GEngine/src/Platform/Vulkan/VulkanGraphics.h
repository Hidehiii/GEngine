#pragma once
#include "GEngine/Renderer/Graphics.h"


namespace GEngine
{
	class VulkanGraphics : public Graphics
	{
	public:
		virtual void Begin() override;
		virtual void End() override;

	private:
		uint32_t m_SwapChainImageIndex = 0;
	};
}


