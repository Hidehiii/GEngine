#include "GEpch.h"
#include "UniformBuffer.h"
#include "GEngine/Graphics/GraphicsAPI.h"
#include "Platform/OpenGL/OpenGLUniformBuffer.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"

namespace GEngine
{
	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
		switch (GraphicsAPI::Current())
		{
		case GraphicsAPI::API::None:    return nullptr;
		case GraphicsAPI::API::OpenGL:  return CreateRef<OpenGLUniformBuffer>(size, binding);
		case GraphicsAPI::API::Vulkan:  return CreateRef<VulkanUniformBuffer>(size, binding);
		}

		GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
		return nullptr;
	}
}