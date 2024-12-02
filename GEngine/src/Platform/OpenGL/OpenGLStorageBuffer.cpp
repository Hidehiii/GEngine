#include "GEpch.h"
#include "OpenGLStorageBuffer.h"
#include <glad/glad.h>

namespace GEngine
{
	OpenGLStorageBuffer::OpenGLStorageBuffer(uint32_t size)
	{
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferStorage(m_RendererID, size, nullptr, GL_DYNAMIC_STORAGE_BIT);
	}
	OpenGLStorageBuffer::~OpenGLStorageBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}
	void OpenGLStorageBuffer::SetData(uint32_t size, const void* data, uint32_t offset)
	{
		
		glNamedBufferSubData(m_RendererID, offset, size, data);
	}
	void OpenGLStorageBuffer::Bind(uint32_t slot)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, m_RendererID);
	}
}

