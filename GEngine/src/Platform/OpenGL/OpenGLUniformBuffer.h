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
		uint32_t m_Binding = 0;
	};
}


