#pragma once
#include "GEngine/Renderer/VertexArray.h"

#include <vulkan/vulkan.h>

namespace GEngine
{
	class GENGINE_API VulkanVertexArray : public VertexArray
	{
	public:
		VulkanVertexArray();
		virtual ~VulkanVertexArray();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetVertexBuffer(const Ref<GEngine::VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const Ref<GEngine::IndexBuffer>& indexBuffer) override;

		virtual const Ref<GEngine::VertexBuffer>& GetVertexBuffer() const override { return m_VertexBuffer; }
		virtual const Ref<GEngine::IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }

		const std::vector<VkVertexInputBindingDescription>& GetVertexInputBindingDescriptions() const { return m_VertexInputBindingDescriptions; }
		const std::vector<VkVertexInputAttributeDescription>& GetVertexInputAttributeDescriptions() const { return m_VertexInputAttributeDescriptions; }
	private:
		uint32_t										m_RendererID;
		uint32_t										m_VertexBufferIndex = 0;
		Ref<GEngine::VertexBuffer>						m_VertexBuffer;
		std::vector<VkVertexInputBindingDescription>	m_VertexInputBindingDescriptions;
		std::vector<VkVertexInputAttributeDescription>	m_VertexInputAttributeDescriptions;
		Ref<GEngine::IndexBuffer>						m_IndexBuffer;
	};
}


