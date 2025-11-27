#include "GEpch.h"
#include "OpenGLBuffer.h"
#include "Platform/OpenGL/OpenGLUtils.h"
#include <glad/glad.h>
namespace GEngine
{
	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size, uint32_t sizeInstance, VertexTopology type)
	{
		

		m_TopologyType = type;
		glCreateBuffers(1, &m_VertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

		glCreateVertexArrays(1, &m_VertexArray);

		if (sizeInstance > 0)
		{
			m_InstanceRendering = true;
			glCreateBuffers(1, &m_InstanceBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeInstance, nullptr, GL_DYNAMIC_DRAW);
		}
	}
	// Vertex Buffer
	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size, uint32_t sizeInstance, VertexTopology type)
	{
		
		m_TopologyType = type;
		glCreateBuffers(1, &m_VertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

		if (sizeInstance > 0)
		{
			m_InstanceRendering = true;
			glCreateBuffers(1, &m_InstanceBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeInstance, nullptr, GL_DYNAMIC_DRAW);
			
		}
	}
	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		glDeleteBuffers(1, &m_VertexBuffer);
		if (m_InstanceRendering)
		{
			glDeleteBuffers(1, &m_InstanceBuffer);
		}
	}
	void OpenGLVertexBuffer::SetData(const void* data, uint32_t size)
	{
		
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}
	void OpenGLVertexBuffer::SetDataInstance(const void* data, uint32_t size)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}


	void OpenGLVertexBuffer::SetLayout(const ShaderInputBufferLayout& layout)
	{
		m_Layout = layout;
		glBindVertexArray(m_VertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);

		uint32_t index = 0;
		for (auto& element : m_Layout)
		{
			switch (element.Type)
			{
			case SHADER_INPUT_DATA_TYPE_FLOAT1:
			case SHADER_INPUT_DATA_TYPE_FLOAT2:
			case SHADER_INPUT_DATA_TYPE_FLOAT3:
			case SHADER_INPUT_DATA_TYPE_FLOAT4:
			{
				if (element.IsInstance)
				{
					glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBuffer);
					glEnableVertexAttribArray(index);
					glVertexAttribPointer(index, element.Size, Utils::ShaderInputDataTypeToGLDataType(element.Type),
						element.Normalized ? GL_TRUE : GL_FALSE, m_Layout.GetStrideInstance(), (const void*)element.Offset);
					glVertexAttribDivisor(index, 1);
				}
				else
				{
					glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
					glEnableVertexAttribArray(index);
					glVertexAttribPointer(index, element.Size, Utils::ShaderInputDataTypeToGLDataType(element.Type),
						element.Normalized ? GL_TRUE : GL_FALSE, m_Layout.GetStride(), (const void*)element.Offset);
				}
				index++;
				break;
			}
			case SHADER_INPUT_DATA_TYPE_INT1:
			case SHADER_INPUT_DATA_TYPE_INT2:
			case SHADER_INPUT_DATA_TYPE_INT3:
			case SHADER_INPUT_DATA_TYPE_INT4:
			{
				if (element.IsInstance)
				{
					glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBuffer);
					glEnableVertexAttribArray(index);
					glVertexAttribIPointer(index, element.Size, Utils::ShaderInputDataTypeToGLDataType(element.Type),
						m_Layout.GetStrideInstance(), (const void*)element.Offset);
					glVertexAttribDivisor(index, 1);
				}
				else
				{
					glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
					glEnableVertexAttribArray(index);
					glVertexAttribIPointer(index, element.Size, Utils::ShaderInputDataTypeToGLDataType(element.Type),
						m_Layout.GetStride(), (const void*)element.Offset);
				}
				index++;
				break;
			}
			default: GE_CORE_ASSERT(false, "Unknown ShaderDataType!");
			}

		}
	}

	void OpenGLVertexBuffer::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		m_IndexBuffer = std::static_pointer_cast<OpenGLIndexBuffer>(indexBuffer);
	}

	void OpenGLVertexBuffer::Bind(CommandBuffer* cmd) const
	{
		glBindVertexArray(m_VertexArray);
		m_IndexBuffer->Bind(cmd);
	}

	// Index Buffer
	OpenGLIndexBuffer::OpenGLIndexBuffer(const uint32_t* indices, uint32_t count)
	{
		m_Count = count;

		glCreateBuffers(1, &m_Buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
	}
	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		glDeleteBuffers(1, &m_Buffer);
	}
	void OpenGLIndexBuffer::Bind(CommandBuffer* cmd) const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffer);
	}
}