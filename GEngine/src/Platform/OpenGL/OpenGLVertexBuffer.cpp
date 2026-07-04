#include "GEpch.h"
#include "OpenGLVertexBuffer.h"
#include "Platform/OpenGL/OpenGLUtils.h"
#include <glad/glad.h>
namespace GEngine
{
	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size, uint32_t sizeInstance, VertexTopology type)
	{
		m_TopologyType = type;
		m_TotalSizeVertex = size;
		glCreateBuffers(1, &m_VertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

		glCreateVertexArrays(1, &m_VertexArray);

		if (sizeInstance > 0)
		{
			m_TotalSizeInstance = sizeInstance;
			glCreateBuffers(1, &m_InstanceBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeInstance, nullptr, GL_DYNAMIC_DRAW);
		}
	}
	// Vertex Buffer
	OpenGLVertexBuffer::OpenGLVertexBuffer(const void* vertices, uint32_t size, uint32_t sizeInstance, VertexTopology type)
	{
		m_TotalSizeVertex = size;
		m_TopologyType = type;
		glCreateBuffers(1, &m_VertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

		if (sizeInstance > 0)
		{
			m_TotalSizeInstance = sizeInstance;
			glCreateBuffers(1, &m_InstanceBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeInstance, nullptr, GL_DYNAMIC_DRAW);
			
		}
		SetVertexData(vertices, size);
	}
	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		glDeleteBuffers(1, &m_VertexBuffer);
		if (m_InstanceBuffer)
		{
			glDeleteBuffers(1, &m_InstanceBuffer);
		}
	}
	void OpenGLVertexBuffer::SetVertexData(const void* data, uint32_t size)
	{
		
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}
	void OpenGLVertexBuffer::SetInstanceData(const void* data, uint32_t size)
	{
		GE_CORE_ASSERT(m_InstanceBuffer, "Instance rendering is not enabled for this vertex buffer.");
		glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}

	void OpenGLVertexBuffer::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		m_IndexBuffer = std::static_pointer_cast<OpenGLIndexBuffer>(indexBuffer);
	}

	uint32_t OpenGLVertexBuffer::GetIndexCount() const
	{
		return m_IndexBuffer != nullptr ? m_IndexBuffer->GetCount() : 0;
	}

	void OpenGLVertexBuffer::Bind(CommandBuffer* cmd) const
	{
		glBindVertexArray(m_VertexArray);
		m_IndexBuffer->Bind(cmd);
	}

	void OpenGLVertexBuffer::SetShaderAndInputLayout(const Ref<Shader>& shader, uint32_t pass)
	{
		GE_CORE_ASSERT(shader, "Shader is null!");
		m_Shader = shader;
		m_ShaderPass = pass;

		glBindVertexArray(m_VertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);

		for (auto& e : m_Shader->GetPassReflections().at(m_ShaderPass).VertexInputs)
		{
			switch (e.Type)
			{
			case SHADER_INPUT_DATA_TYPE_FLOAT1:
			case SHADER_INPUT_DATA_TYPE_FLOAT2:
			case SHADER_INPUT_DATA_TYPE_FLOAT3:
			case SHADER_INPUT_DATA_TYPE_FLOAT4:
			{
				if (e.IsPerInstance)
				{
					glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBuffer);
					glEnableVertexAttribArray(e.Location);
					glVertexAttribPointer(e.Location, Utils::ShaderInputDataSize(e.Type), 
						Utils::ShaderInputDataTypeToGLDataType(e.Type), GL_FALSE, 
						m_Shader->GetPassReflections().at(m_ShaderPass).VertexInputInstanceStride, (const void*)e.Offset);
					glVertexAttribDivisor(e.Location, 1);
				}
				else
				{
					glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
					glEnableVertexAttribArray(e.Location);
					glVertexAttribPointer(e.Location, Utils::ShaderInputDataSize(e.Type),
						Utils::ShaderInputDataTypeToGLDataType(e.Type), GL_FALSE,
						m_Shader->GetPassReflections().at(m_ShaderPass).VertexInputVertexStride, (const void*)e.Offset);
				}
				break;
			}
			case SHADER_INPUT_DATA_TYPE_INT1:
			case SHADER_INPUT_DATA_TYPE_INT2:
			case SHADER_INPUT_DATA_TYPE_INT3:
			case SHADER_INPUT_DATA_TYPE_INT4:
			case SHADER_INPUT_DATA_TYPE_UINT1:
			case SHADER_INPUT_DATA_TYPE_UINT2:
			case SHADER_INPUT_DATA_TYPE_UINT3:
			case SHADER_INPUT_DATA_TYPE_UINT4:
			{
				if (e.IsPerInstance)
				{
					glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBuffer);
					glEnableVertexAttribArray(e.Location);
					glVertexAttribIPointer(e.Location, Utils::ShaderInputDataSize(e.Type), 
						Utils::ShaderInputDataTypeToGLDataType(e.Type),
						m_Shader->GetPassReflections().at(m_ShaderPass).VertexInputInstanceStride, (const void*)e.Offset);
					glVertexAttribDivisor(e.Location, 1);
				}
				else
				{
					glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
					glEnableVertexAttribArray(e.Location);
					glVertexAttribIPointer(e.Location, Utils::ShaderInputDataSize(e.Type),
						Utils::ShaderInputDataTypeToGLDataType(e.Type),
						m_Shader->GetPassReflections().at(m_ShaderPass).VertexInputVertexStride, (const void*)e.Offset);
				}
				break;
			}
			default: GE_CORE_ASSERT(false, "Unknown ShaderDataType!");
			}
		}
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