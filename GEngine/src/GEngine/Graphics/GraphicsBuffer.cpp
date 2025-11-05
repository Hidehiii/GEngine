#include "GEpch.h"
#include "GraphicsBuffer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Graphics.h"

namespace GEngine
{
	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size, uint32_t sizeInstance, VertexTopology type)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_NONE: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GRAPHICS_API_OPENGL: {
			return CreateRef<OpenGLVertexBuffer>(size, sizeInstance, type);
		}
		case GRAPHICS_API_VULKAN: {
			return CreateRef<VulkanVertexBuffer>(size, sizeInstance, type);
		}
		case GRAPHICS_API_DIRECT3DX12: {
			GE_CORE_ASSERT(false, "GraphicsAPI::Direct3DX12 is currently not supported!");
			return nullptr;
		}
		}
		GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size, uint32_t sizeInstance, VertexTopology type)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_NONE: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
			}
		case GRAPHICS_API_OPENGL: {
			return CreateRef<OpenGLVertexBuffer>(vertices, size, sizeInstance, type);
			}
		case GRAPHICS_API_VULKAN: {
			return CreateRef<VulkanVertexBuffer>(vertices, size, sizeInstance, type);
		}
		case GRAPHICS_API_DIRECT3DX12: {
			GE_CORE_ASSERT(false, "GraphicsAPI::Direct3DX12 is currently not supported!");
			return nullptr;
		}
		}
		
		GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(const uint32_t* indices, uint32_t count)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_NONE: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
			}
		case GRAPHICS_API_OPENGL: {
			return CreateRef<OpenGLIndexBuffer>(indices, count);
			}
		case GRAPHICS_API_VULKAN: {
			return CreateRef<VulkanIndexBuffer>(indices, count);
		}
		case GRAPHICS_API_DIRECT3DX12: {
			GE_CORE_ASSERT(false, "GraphicsAPI::Direct3DX12 is currently not supported!");
			return nullptr;
		}
		}

		GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
		return nullptr;
	}

	void ShaderInputBufferLayout::CalculateOffsetsAndStride()
	{
		uint32_t offset = 0;
		m_Stride = 0;
		m_StrideInstance = 0;
		for (auto& element : m_Datas)
		{
			if (element.IsInstance == false)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}
		offset = 0;
		for (auto& element : m_Datas)
		{
			if (element.IsInstance)
			{
				element.Offset = offset;
				offset += element.Size;
				m_StrideInstance += element.Size;
			}
		}
	}
}