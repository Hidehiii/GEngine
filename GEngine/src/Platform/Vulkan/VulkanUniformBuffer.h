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
	private:
		VkBuffer					m_UniformBuffer;
		VkDeviceMemory				m_UniformBufferMemory;
		void*						m_MapData;
		uint32_t					m_Offset = 0;
		int							m_Binding;
		VkDescriptorSetLayoutBinding m_DescriptorSetLayoutBinding;

	public:
		static void AddDescriptorSetLayoutBinding(VkDescriptorSetLayoutBinding layoutBinding);
		static void RemoveDescriptorSetLayoutBinding(VkDescriptorSetLayoutBinding layoutBinding);
		static VkDescriptorSetLayout GetDescriptorSetLayout() { return s_DescriptorSetLayout; }
		static VkDescriptorSet GetDescriptorSet() { return s_DescriptorSet; }
	private:
		static void CreateDescriptorSet();
		static void CreateDescriptorSetLayout();
	private:
		static std::vector<VkDescriptorSetLayoutBinding> s_DescriptorSetLayoutBindings;
		static VkDescriptorSetLayout	s_DescriptorSetLayout;
		static VkDescriptorSet			s_DescriptorSet;
	};
}



