#include "GEpch.h"
#include "OpenGLBuffer.h"
#include <glad/glad.h>
namespace GEngine
{
	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size, uint32_t sizeInstance, VertexTopology type)
	{
		GE_PROFILE_FUNCTION();

		m_TopologyType = type;
		glCreateBuffers(1, &m_VertexBufferRendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferRendererID);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

		glCreateVertexArrays(1, &m_VertexArrayRendererID);

		if (sizeInstance > 0)
		{
			m_InstanceRendering = true;
			glCreateBuffers(1, &m_InstanceBufferRendererID);
			glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBufferRendererID);
			glBufferData(GL_ARRAY_BUFFER, sizeInstance, nullptr, GL_DYNAMIC_DRAW);
		}
	}
	// Vertex Buffer
	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size, uint32_t sizeInstance, VertexTopology type)
	{
		GE_PROFILE_FUNCTION();
		m_TopologyType = type;
		glCreateBuffers(1, &m_VertexBufferRendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferRendererID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

		if (sizeInstance > 0)
		{
			m_InstanceRendering = true;
			glCreateBuffers(1, &m_InstanceBufferRendererID);
			glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBufferRendererID);
			glBufferData(GL_ARRAY_BUFFER, sizeInstance, nullptr, GL_DYNAMIC_DRAW);
			
		}
	}
	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		glDeleteBuffers(1, &m_VertexBufferRendererID);
		if (m_InstanceRendering)
		{
			glDeleteBuffers(1, &m_InstanceBufferRendererID);
		}
	}
	void OpenGLVertexBuffer::SetData(const void* data, uint32_t size)
	{
		GE_PROFILE_FUNCTION();
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferRendererID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}
	void OpenGLVertexBuffer::SetDataInstance(const void* data, uint32_t size)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBufferRendererID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}
	void OpenGLVertexBuffer::Bind() const
	{
		GE_PROFILE_FUNCTION();
		glBindVertexArray(m_VertexArrayRendererID);
		//glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferRendererID);
	}
	void OpenGLVertexBuffer::Unbind() const
	{
		GE_PROFILE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void OpenGLVertexBuffer::SetLayout(const BufferLayout& layout)
	{
		m_Layout = layout;
		glBindVertexArray(m_VertexArrayRendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferRendererID);

		uint32_t index = 0;
		for (auto& element : m_Layout)
		{
			switch (element.Type)
			{
			case ShaderDataType::float1:
			case ShaderDataType::float2:
			case ShaderDataType::float3:
			case ShaderDataType::float4:
			{
				if (element.IsInstance)
				{
					glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBufferRendererID);
					glEnableVertexAttribArray(index);
					glVertexAttribPointer(index, element.GetElementDataSize(), ShaderDataTypeToOpenGLBaseType(element.Type),
						element.Normalized ? GL_TRUE : GL_FALSE, m_Layout.GetStrideInstance(), (const void*)element.Offset);
					glVertexAttribDivisor(index, 1);
				}
				else
				{
					glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferRendererID);
					glEnableVertexAttribArray(index);
					glVertexAttribPointer(index, element.GetElementDataSize(), ShaderDataTypeToOpenGLBaseType(element.Type),
						element.Normalized ? GL_TRUE : GL_FALSE, m_Layout.GetStride(), (const void*)element.Offset);
				}
				index++;
				break;
			}
			case ShaderDataType::int1:
			case ShaderDataType::int2:
			case ShaderDataType::int3:
			case ShaderDataType::int4:
			{
				if (element.IsInstance)
				{
					glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBufferRendererID);
					glEnableVertexAttribArray(index);
					glVertexAttribIPointer(index, element.GetElementDataSize(), ShaderDataTypeToOpenGLBaseType(element.Type),
						m_Layout.GetStrideInstance(), (const void*)element.Offset);
					glVertexAttribDivisor(index, 1);
				}
				else
				{
					glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferRendererID);
					glEnableVertexAttribArray(index);
					glVertexAttribIPointer(index, element.GetElementDataSize(), ShaderDataTypeToOpenGLBaseType(element.Type),
						m_Layout.GetStride(), (const void*)element.Offset);
				}
				index++;
				break;
			}
			// temporary has some issues
			// 之后需要看看怎么把mat3和mat4的数据传进顶点属性
			case ShaderDataType::mat3:
			case ShaderDataType::mat4:
			{
				GE_CORE_ASSERT(false, "mat3 and mat4 is not supported yet!");
				for (uint8_t i = 0; i < element.GetElementDataSize(); i++)
				{
					glEnableVertexAttribArray(index);
					glVertexAttribPointer(index, element.GetElementDataSize(), ShaderDataTypeToOpenGLBaseType(element.Type),
						element.Normalized ? GL_TRUE : GL_FALSE, m_Layout.GetStride(), (const void*)(element.Offset + sizeof(float) * i * element.GetElementDataSize()));
					glVertexAttribDivisor(index, 1);
					index++;
				}
				break;
			}
			default: GE_CORE_ASSERT(false, "Unknown ShaderDataType!");
			}

		}
	}

	void OpenGLVertexBuffer::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		glBindVertexArray(m_VertexArrayRendererID);
		indexBuffer->Bind();
		m_IndexBuffer = indexBuffer;
	}

	// Index Buffer
	OpenGLIndexBuffer::OpenGLIndexBuffer(const uint32_t* indices, uint32_t count)
		: m_Count(count)
	{
		GE_PROFILE_FUNCTION();

		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
	}
	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}
	void OpenGLIndexBuffer::Bind() const
	{
		GE_PROFILE_FUNCTION();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}
	void OpenGLIndexBuffer::Unbind() const
	{
		GE_PROFILE_FUNCTION();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}