#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/FrameBuffer.h"
#include <vulkan/vulkan.h>

namespace GEngine
{
	struct VulkanRenderPassSpecification
	{
		std::vector<FrameBufferTextureFormat>	ColorAttachmentFormats;
		FrameBufferTextureFormat				DepthAttachmentFormat;
	};

	class GENGINE_API VulkanRenderPass
	{
	public:
		VkRenderPass GetRenderPass() { return m_RenderPass; }
	private:
		void CreateRenderPass(const VulkanRenderPassSpecification& spec);

		
	private:
		VkRenderPass						m_RenderPass;
	};
}

