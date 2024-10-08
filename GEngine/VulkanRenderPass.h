#pragma once
#include "GEngine/Core/Core.h"

#include <vulkan/vulkan.h>

namespace GEngine
{
	struct VulkanRenderPassSpecification
	{
		VkFormat SwapChainImageFormat;
	};

	class GENGINE_API VulkanRenderPass
	{
	public:

	private:
		void CreateRenderPass(const VulkanRenderPassSpecification& spec);
	private:
		VkRenderPass						m_RenderPass;
	};
}


