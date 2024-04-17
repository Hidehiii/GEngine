#include "GEpch.h"
#include "OpenGLVertexArray.h"


namespace GEngine
{
	OpenGLVertexArray::OpenGLVertexArray()
	{
		GE_PROFILE_FUNCTION();

		glCreateVertexArrays(1, &m_RendererID);
	}
	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_RendererID);
	}
	void OpenGLVertexArray::Bind() const
	{
		GE_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
	}
	void OpenGLVertexArray::Unbind() const
	{
		GE_PROFILE_FUNCTION();

		glBindVertexArray(0);
	}
	void OpenGLVertexArray::AddVertexBuffer(const Ref<GEngine::VertexBuffer>& vertexBuffer)
	{
		GE_PROFILE_FUNCTION();

		GE_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");


		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();


		uint32_t index = 0;
		for (auto& element : vertexBuffer->GetLayout())
		{
			switch (element.Type)
			{
				case ShaderDataType::float1:
				case ShaderDataType::float2:
				case ShaderDataType::float3:
				case ShaderDataType::float4:
				{
					glEnableVertexAttribArray(index);
					glVertexAttribPointer(index, element.GetComponentCount(), ShaderDataTypeToOpenGLBaseType(element.Type),
						element.Normalized ? GL_TRUE : GL_FALSE, vertexBuffer->GetLayout().GetStride(), (const void*)element.Offset);
					index++;
					break;
				}
				case ShaderDataType::int1:
				case ShaderDataType::int2:
				case ShaderDataType::int3:
				case ShaderDataType::int4:
				{
					glEnableVertexAttribArray(index);
					glVertexAttribIPointer(index, element.GetComponentCount(), ShaderDataTypeToOpenGLBaseType(element.Type),
						vertexBuffer->GetLayout().GetStride(), (const void*)element.Offset);
					index++;
					break;
				}
				case ShaderDataType::mat3:
				case ShaderDataType::mat4:
				{
					for (uint8_t i = 0; i < element.GetComponentCount(); i++)
					{
						glEnableVertexAttribArray(index);
						glVertexAttribPointer(index, element.GetComponentCount(), ShaderDataTypeToOpenGLBaseType(element.Type),
											element.Normalized ? GL_TRUE : GL_FALSE, vertexBuffer->GetLayout().GetStride(), (const void*)(element.Offset + sizeof(float) * i * element.GetComponentCount()));
						glVertexAttribDivisor(index, 1);
						index++;
					}
					break;
				}
				default: GE_CORE_ASSERT(false, "Unknown ShaderDataType!");
			}
			
		}
		m_VertexBuffers.push_back(vertexBuffer);
	}
	void OpenGLVertexArray::SetIndexBuffer(const Ref<GEngine::IndexBuffer>& indexBuffer)
	{
		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();
		m_IndexBuffer = indexBuffer;
	}
}