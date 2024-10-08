#include "GEpch.h"
#include "VulkanVertexArray.h"



namespace GEngine
{
	VulkanVertexArray::VulkanVertexArray()
	{
	}
	VulkanVertexArray::~VulkanVertexArray()
	{
	}
	void VulkanVertexArray::Bind() const
	{
		
	}
	void VulkanVertexArray::Unbind() const
	{
	}
	void VulkanVertexArray::AddVertexBuffer(const Ref<GEngine::VertexBuffer>& vertexBuffer)
	{
		GE_PROFILE_FUNCTION();

		GE_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

		VkVertexInputBindingDescription		bindingDescription{};
		bindingDescription.binding			= static_cast<uint32_t>(m_VertexInputBindingDescriptions.size());
		bindingDescription.stride			= vertexBuffer->GetLayout().GetStride();
		bindingDescription.inputRate		= VK_VERTEX_INPUT_RATE_VERTEX;

		uint32_t index = 0;
		for (auto& element : vertexBuffer->GetLayout())
		{
			VkVertexInputAttributeDescription		attributeDescription{};
			attributeDescription.binding			= static_cast<uint32_t>(m_VertexInputBindingDescriptions.size());
			attributeDescription.location			= index;
			attributeDescription.offset				= element.Offset;
			switch (element.Type)
			{
			case ShaderDataType::float1:
				attributeDescription.format			= VK_FORMAT_R32_SFLOAT;
				break;
			case ShaderDataType::float2:
				attributeDescription.format			= VK_FORMAT_R32G32_SFLOAT;
				break;
			case ShaderDataType::float3:
				attributeDescription.format			= VK_FORMAT_R32G32B32_SFLOAT;
				break;
			case ShaderDataType::float4:
				attributeDescription.format			= VK_FORMAT_R32G32B32A32_SFLOAT;
				break;
			default:
				GE_CORE_ASSERT(false, "Unkonw Shader Date Type");
				break;
			}
		}

		m_VertexInputBindingDescriptions.push_back(bindingDescription);
	}
	void VulkanVertexArray::SetIndexBuffer(const Ref<GEngine::IndexBuffer>& indexBuffer)
	{
		m_IndexBuffer = indexBuffer;
	}
}