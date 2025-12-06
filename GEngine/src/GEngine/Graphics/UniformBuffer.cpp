#include "GEpch.h"
#include "UniformBuffer.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/OpenGL/OpenGLUniformBuffer.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"

namespace GEngine
{
	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t count)
	{
		Ref<UniformBuffer>	buf;

		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_NONE:    GE_CORE_ASSERT(false, "Unknown GraphicsAPI!"); break;
		case GRAPHICS_API_OPENGL:  buf = CreateRef<OpenGLUniformBuffer>(size, count); break;
		case GRAPHICS_API_VULKAN:  buf = CreateRef<VulkanUniformBuffer>(size, count); break;
		}
		
		return buf;
	}
}