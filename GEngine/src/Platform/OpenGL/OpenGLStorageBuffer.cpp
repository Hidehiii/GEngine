#include "GEpch.h"
#include "OpenGLStorageBuffer.h"
#include <glad/glad.h>

namespace GEngine
{
	OpenGLStorageBuffer::OpenGLStorageBuffer(uint32_t size, uint32_t binding)
	{
		m_Binding = binding;
		glGenBuffers(1, &m_RendererID);
		glNamedBufferStorage(m_RendererID, size, nullptr, GL_DYNAMIC_STORAGE_BIT);
	}
	void OpenGLStorageBuffer::SetData(uint32_t size, const void* data, uint32_t offset)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_Binding, m_RendererID);
		glNamedBufferSubData(m_RendererID, offset, size, data);
	}
}

