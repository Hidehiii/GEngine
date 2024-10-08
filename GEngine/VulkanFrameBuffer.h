#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/FrameBuffer.h"
#include "VulkanRenderPass.h"

namespace GEngine
{
	class GENGINE_API VulkanFrameBuffer : public FrameBuffer
	{
	public:
		VulkanFrameBuffer(const FrameBufferSpecification& spec);
	private:
		Ref<VulkanRenderPass>		m_RenderPass = nullptr;
	};
}


