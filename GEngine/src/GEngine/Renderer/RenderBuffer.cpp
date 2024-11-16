#include "GEpch.h"
#include "RenderBuffer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Renderer.h"

namespace GEngine
{
	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size, VertexTopology type)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: {
			GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		}
		case RendererAPI::API::OpenGL: {
			return CreateRef<OpenGLVertexBuffer>(size, type);
		}
		case RendererAPI::API::Vulkan: {
			return CreateRef<VulkanVertexBuffer>(size, type);
		}
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size, VertexTopology type)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: {
			GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
			}
		case RendererAPI::API::OpenGL: {
			return CreateRef<OpenGLVertexBuffer>(vertices, size, type);
			}
		case RendererAPI::API::Vulkan: {
			return CreateRef<VulkanVertexBuffer>(vertices, size, type);
		}
		}
		
		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: {
			GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
			}
		case RendererAPI::API::OpenGL: {
			return CreateRef<OpenGLIndexBuffer>(indices, count);
			}
		case RendererAPI::API::Vulkan: {
			return CreateRef<VulkanIndexBuffer>(indices, count);
		}
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	void BufferLayout::CalculateOffsetsAndStride()
	{
		uint32_t offset = 0;
		m_Stride = 0;
		for (auto& element : m_Elements)
		{
			element.Offset = offset;
			offset += element.Size;
			m_Stride += element.Size;
		}
	}
}