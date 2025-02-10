#include "GEpch.h"
#include "OpenGLUniformBuffer.h"
#include <glad/glad.h>

namespace GEngine
{
	OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t binding)
	{
		glCreateBuffers(1, &m_UniformBuffer);
		glNamedBufferData(m_UniformBuffer, size, nullptr, GL_DYNAMIC_DRAW); // TODO: investigate usage hint
		m_Binding = binding;
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		glDeleteBuffers(1, &m_UniformBuffer);
	}


	void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		glBindBufferRange(GL_UNIFORM_BUFFER, m_Binding, m_UniformBuffer, offset, size);
		glNamedBufferSubData(m_UniformBuffer, offset, size, data);
	}
}