#include "GEpch.h"
#include "OpenGLUniformBuffer.h"
#include "GEngine/Graphics/Graphics.h"
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


	OpenGLUniformBufferDynamic::OpenGLUniformBufferDynamic(uint32_t size, uint32_t count, uint32_t binding, uint32_t globalIndex)
	{
		

		m_Binding				= binding;
		uint32_t minUboAligment = Graphics::GetMinUniformBufferOffsetAlignment();
		m_Aligment				= (size + minUboAligment - 1) & ~(minUboAligment - 1);
		m_TotalSize				= m_Aligment * count;
		m_GlobalIndex			= globalIndex;

		glCreateBuffers(1, &m_UniformBuffer);
		glNamedBufferData(m_UniformBuffer, m_TotalSize, nullptr, GL_DYNAMIC_DRAW); // TODO: investigate usage hint
	}
	OpenGLUniformBufferDynamic::~OpenGLUniformBufferDynamic()
	{
		glDeleteBuffers(1, &m_UniformBuffer);
	}
	void OpenGLUniformBufferDynamic::SetData(const void* data, uint32_t size)
	{
		GE_CORE_ASSERT(size <= m_Aligment, "");
		m_Offset = m_Offset + m_Aligment < m_TotalSize ? m_Offset + m_Aligment : 0;

		glBindBufferRange(GL_UNIFORM_BUFFER, m_Binding, m_UniformBuffer, m_Offset, size);
		glNamedBufferSubData(m_UniformBuffer, m_Offset, size, data);

		UpdateGlobalUniformOffset();
	}
}