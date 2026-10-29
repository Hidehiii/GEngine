#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/UniformBuffer.h"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace GEngine
{
	class GENGINE_API VulkanUniformBuffer : public UniformBuffer
	{
	public:
		VulkanUniformBuffer(uint32_t size, uint32_t count = 1, bool autoSetDataDynamic = true);
		virtual ~VulkanUniformBuffer();

		virtual void SetData(const void* data, uint32_t size) override;
		virtual void SetDataDynamic(const void* data, uint32_t size) override;

		VkDescriptorBufferInfo* GetDescriptorBufferInfo() { return &m_BufferInfo; }
		VkDescriptorType		GetDescriptorType() const { return m_DescriptorType; }

		bool operator==(const VulkanUniformBuffer& other) const
		{
			return m_UniformBuffer == other.m_UniformBuffer;
		}
	private:
		VkBuffer					m_UniformBuffer = nullptr;
		VmaAllocation				m_UniformBufferAllocation = nullptr;
		void*						m_MapData = nullptr;
		VkDescriptorBufferInfo		m_BufferInfo{};
		VkDescriptorType			m_DescriptorType;
	};

}



