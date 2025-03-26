#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/UniformBuffer.h"

namespace GEngine
{
	class GENGINE_API OpenGLUniformBuffer : public UniformBuffer
	{
	public:
		OpenGLUniformBuffer(uint32_t size, uint32_t binding);
		virtual ~OpenGLUniformBuffer();

		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
	private:
		uint32_t m_UniformBuffer = 0;
		
	};


	class GENGINE_API OpenGLUniformBufferDynamic : public UniformBufferDynamic
	{
	public:
		OpenGLUniformBufferDynamic(uint32_t size, uint32_t count, uint32_t binding, uint32_t globalIndex = -1);
		virtual ~OpenGLUniformBufferDynamic();

		virtual void SetData(const void* data, uint32_t size) override;
	private:
		uint32_t m_UniformBuffer = 0;

	};
}


