#include "GEpch.h"
#include "UniformBuffer.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/OpenGL/OpenGLUniformBuffer.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"

namespace GEngine
{
	std::vector<Ref<UniformBufferDynamic>>			UniformBufferDynamic::s_GlobalUniforms;
	std::vector<uint32_t>							UniformBufferDynamic::s_GlobalUniformOffsets;

	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
		Ref<UniformBuffer>	buf;

		switch (Graphics::GetGraphicsAPI())
		{
		case GraphicsAPI::GRAPHICS_API_None:    GE_CORE_ASSERT(false, "Unknown GraphicsAPI!"); break;
		case GraphicsAPI::GRAPHICS_API_OpenGL:  buf = CreateRef<OpenGLUniformBuffer>(size, binding); break;
		case GraphicsAPI::GRAPHICS_API_Vulkan:  buf = CreateRef<VulkanUniformBuffer>(size, binding); break;
		}
		
		return buf;
	}


	Ref<UniformBufferDynamic> UniformBufferDynamic::Create(uint32_t size, uint32_t count, uint32_t binding, bool global)
	{
		Ref<UniformBufferDynamic>	buf;

		switch (Graphics::GetGraphicsAPI())
		{
		case GraphicsAPI::GRAPHICS_API_None:    GE_CORE_ASSERT(false, "Unknown GraphicsAPI!"); break;
		case GraphicsAPI::GRAPHICS_API_OpenGL:  buf = CreateRef<OpenGLUniformBufferDynamic>(size, count, binding, global ? s_GlobalUniforms.size() : -1); break;
		case GraphicsAPI::GRAPHICS_API_Vulkan:  buf = CreateRef<VulkanUniformBufferDynamic>(size, count, binding, global ? s_GlobalUniforms.size() : -1); break;
		}

		if (global)
		{
			s_GlobalUniforms.push_back(buf);
			s_GlobalUniformOffsets.push_back(0);
		}

		return buf;
	}

	void UniformBufferDynamic::UpdateGlobalUniformOffset()
	{
		if (m_GlobalIndex >= 0)
		{
			s_GlobalUniformOffsets.at(m_GlobalIndex) = m_Offset;
		}
	}
}