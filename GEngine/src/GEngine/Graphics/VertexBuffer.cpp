#include "GEpch.h"
#include "VertexBuffer.h"
#include "GEngine/Graphics/Shader.h"
#include "Platform/OpenGL/OpenGLVertexBuffer.h"
#include "Platform/Vulkan/VulkanVertexBuffer.h"
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

	Ref<VertexBuffer> VertexBuffer::Create(const void* vertices, uint32_t size, uint32_t sizeInstance, VertexTopology type)
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

	bool VertexBuffer::IsInstanceRendering() const
	{
		GE_CORE_ASSERT(m_Shader != nullptr, "Shader is not set for this vertex buffer.");
		return m_Shader->GetPassReflections().at(m_ShaderPass).VertexInputInstanceStride > 0;
	}

	uint32_t VertexBuffer::GetVertexCount() const
	{
		GE_CORE_ASSERT(m_Shader != nullptr, "Shader is not set for this vertex buffer.");
		GE_CORE_ASSERT(m_Shader->GetPassReflections().at(m_ShaderPass).VertexInputVertexStride != 0, "VertexInputVertexStride is zero, cannot calculate vertex count.");
		return m_TotalSizeVertex / m_Shader->GetPassReflections().at(m_ShaderPass).VertexInputVertexStride;
	}

	uint32_t VertexBuffer::GetInstanceCount() const
	{
		GE_CORE_ASSERT(m_Shader != nullptr, "Shader is not set for this vertex buffer.");
		GE_CORE_ASSERT(m_Shader->GetPassReflections().at(m_ShaderPass).VertexInputInstanceStride != 0, "VertexInputInstanceStride is zero, cannot calculate instance count.");
		return m_TotalSizeInstance > 0 ? m_TotalSizeInstance / m_Shader->GetPassReflections().at(m_ShaderPass).VertexInputInstanceStride : 1;
	}
}