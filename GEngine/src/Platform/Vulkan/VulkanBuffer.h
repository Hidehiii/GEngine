#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/RenderBuffer.h"


#include <vulkan/vulkan.h>

namespace GEngine
{
	class GENGINE_API VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(uint32_t size);
		VulkanVertexBuffer(float* vertices, uint32_t size);
		virtual ~VulkanVertexBuffer();

		virtual void SetData(const void* data, uint32_t size) override;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
		virtual const BufferLayout& GetLayout() const override { return m_Layout; }
	private:
		VkBuffer		m_VertexBuffer;
		VkDeviceMemory	m_VertexBufferMemory;
		BufferLayout	m_Layout;
		uint32_t		m_Offset = 0;
	};

	class GENGINE_API VulkanIndexBuffer : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(uint32_t* indices, uint32_t count);
		virtual ~VulkanIndexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual uint32_t GetCount() const override { return m_Count; };
	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
		VkBuffer		m_IndexBuffer;
		VkDeviceMemory	m_IndexBufferMemory;
	};


	namespace Utils
	{
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
		{
			VkPhysicalDeviceMemoryProperties    memProperties;
			vkGetPhysicalDeviceMemoryProperties(VulkanContext::GetPhysicalDevice(), &memProperties);
			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
				if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
					return i;
				}
			}

			GE_CORE_ERROR("failed to find suitable memory type!");
			return 0;
		}

		void CreateBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
			VkBufferCreateInfo      bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
			bufferInfo.usage = usage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VK_CHECK_RESULT(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer));

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

			VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory));

			vkBindBufferMemory(device, buffer, bufferMemory, 0);
		}
	}
}



