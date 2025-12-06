#include "GEpch.h"
#include "OpenGLUniformBuffer.h"
#include "GEngine/Graphics/Graphics.h"
#include <glad/glad.h>

namespace GEngine
{
	OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t count)
	{
		if(count > 1)
		{
			uint32_t minUboAligment = Graphics::GetMinUniformBufferOffsetAlignment();
			m_Aligment				= (size + minUboAligment - 1) & ~(minUboAligment - 1);
			size					= m_Aligment * count;
			m_TotalSize				= size;
		}

		glCreateBuffers(1, &m_UniformBuffer);
		glNamedBufferData(m_UniformBuffer, size, nullptr, GL_DYNAMIC_DRAW); // TODO: investigate usage hint
		
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		glDeleteBuffers(1, &m_UniformBuffer);
	}


	void OpenGLUniformBuffer::SetData(const void* data, uint32_t size)
	{
		GE_CORE_ASSERT(m_Binding != UINT32_MAX, "Uniform buffer binding point is not set!");
		if(m_TotalSize > 0)
		{
			GE_CORE_ASSERT(size <= m_Aligment, "");
			m_Offset = m_Offset + m_Aligment < m_TotalSize ? m_Offset + m_Aligment : 0;
		}
		glBindBufferRange(GL_UNIFORM_BUFFER, m_Binding, m_UniformBuffer, m_Offset, size);
		glNamedBufferSubData(m_UniformBuffer, m_Offset, size, data);
	}

	void OpenGLUniformBuffer::SetBindPoint(uint32_t binding)
	{
		m_Binding = binding;
	}

}