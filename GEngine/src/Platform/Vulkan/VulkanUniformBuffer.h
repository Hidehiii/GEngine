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

		VkDescriptorSetLayoutBinding GetDescriptorSetLayoutBinding() { return m_DescriptorSetLayoutBinding; }
		VkDescriptorBufferInfo GetDescriptorBufferInfo() { return m_BufferInfo; }
		int GetBinding() { return m_Binding; }
	private:
		VkBuffer					m_UniformBuffer;
		VkDeviceMemory				m_UniformBufferMemory;
		void*						m_MapData;
		uint32_t					m_Offset = 0;
		int							m_Binding;
		VkDescriptorBufferInfo		m_BufferInfo{};
		VkDescriptorSetLayoutBinding m_DescriptorSetLayoutBinding;

	public:
		static void AddPublicUniformBuffer(VulkanUniformBuffer* buffer);
		static void RemovePublicUniformBuffer(VulkanUniformBuffer* buffer);
		static std::vector<VulkanUniformBuffer*> GetPublicUniformBuffer() { return s_PublicUniformBuffer; }
	private:
		static std::vector<VulkanUniformBuffer*> s_PublicUniformBuffer;
	};
}



