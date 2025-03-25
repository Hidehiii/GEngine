#pragma once
#include "GEngine/Graphics/GraphicsPresent.h"
#include <vulkan/vulkan.h>

namespace GEngine
{

	class VulkanCommandBuffer;

	class GENGINE_API VulkanGraphicsPresent : public GraphicsPresent
	{
	public:
		VulkanGraphicsPresent();
		virtual bool AquireImage() override;
		virtual void Begin() override;
		virtual void End() override;

	private:
		uint32_t							m_SwapChainImageIndex = 0;
		bool								m_RecreateSwapChain = false;
		std::vector<VkFence>				m_Fences;
	};
}


