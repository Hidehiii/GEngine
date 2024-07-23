#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/UniformBuffer.h"

namespace GEngine
{
	class GENGINE_API VulkanUniformBuffer : public UniformBuffer
	{
	public:
		VulkanUniformBuffer(uint32_t size, uint32_t binding);
		virtual ~VulkanUniformBuffer();

		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
		virtual void RT_SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
	};
}


