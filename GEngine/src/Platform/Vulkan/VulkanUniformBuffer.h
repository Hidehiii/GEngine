#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/UniformBuffer.h"
#include <vulkan/vulkan.h>

namespace GEngine
{
	class GENGINE_API VulkanUniformBuffer : public UniformBuffer
	{
	public:
		VulkanUniformBuffer(uint32_t size, uint32_t binding);
		virtual ~VulkanUniformBuffer();

		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
		virtual void RT_SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

	private:
		VkDescriptorSetLayout		m_DescriptorSetLayout;
		VkBuffer					m_UniformBuffer;
		VkDeviceMemory				m_UniformBufferMemory;
		void*						m_MapData;
		uint32_t					m_Offset = 0;
	};
}


