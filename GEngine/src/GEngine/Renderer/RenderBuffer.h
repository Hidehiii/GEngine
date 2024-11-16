#pragma once

#include "GEngine/Core/Core.h"
#include <glad/glad.h>

namespace GEngine
{
	enum class ShaderDataType
	{
		None,
		float1,
		float2,
		float3,
		float4,
		int1,
		int2,
		int3,
		int4,
		mat3,
		mat4,
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::float1: return 4;
		case ShaderDataType::float2: return 4 * 2;
		case ShaderDataType::float3: return 4 * 3;
		case ShaderDataType::float4: return 4 * 4;
		case ShaderDataType::int1:   return 4;
		case ShaderDataType::int2:   return 4 * 2;
		case ShaderDataType::int3:   return 4 * 3;
		case ShaderDataType::int4:   return 4 * 4;
		case ShaderDataType::mat3:   return 4 * 3 * 3;
		case ShaderDataType::mat4:   return 4 * 4 * 4;
		}
		GE_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}
	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::float1: return GL_FLOAT;
			case ShaderDataType::float2: return GL_FLOAT;
			case ShaderDataType::float3: return GL_FLOAT;
			case ShaderDataType::float4: return GL_FLOAT;
			case ShaderDataType::int1:   return GL_INT;
			case ShaderDataType::int2:   return GL_INT;
			case ShaderDataType::int3:   return GL_INT;
			case ShaderDataType::int4:   return GL_INT;
			case ShaderDataType::mat3:   return GL_FLOAT;
			case ShaderDataType::mat4:   return GL_FLOAT;
		}
		GE_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	// Buffer Element
	// contain the name, type, offset, size and normalized
	struct GENGINE_API BufferElement
	{
		// Name of the buffer element
		std::string Name;
		// Type of the buffer element
		ShaderDataType Type;
		// Offset of the buffer element
		// Offset is the offset of the buffer element in the buffer
		// For example
		// {float3 position, float2 texCoord}
		// the offset of position is 0
		// the offset of texCoord is 3 * 4 = 12
		uint32_t Offset;
		// Size of the buffer element
		// Size is the size of the buffer element
		// For example, float1 is 4 bytes, float2 is 4 * 2 bytes
		uint32_t Size;
		// Is the buffer element normalized
		bool Normalized;
		// Is instance data
		bool IsInstance;

		BufferElement() {}

		BufferElement(ShaderDataType type, const std::string& name, bool isInstance = false, bool normalized = false)
			: Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), IsInstance(isInstance), Normalized(normalized){}

		uint32_t GetElementDataSize() const
		{
			switch (Type) {
				case ShaderDataType::float1: return 1;
				case ShaderDataType::float2: return 2;
				case ShaderDataType::float3: return 3;
				case ShaderDataType::float4: return 4;
				case ShaderDataType::int1:   return 1;
				case ShaderDataType::int2:   return 2;
				case ShaderDataType::int3:   return 3;
				case ShaderDataType::int4:   return 4;
				case ShaderDataType::mat3:   return 3 * 3;
				case ShaderDataType::mat4:   return 4 * 4;
			}
			GE_CORE_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}
	};

	enum class VertexTopology
	{
		Point		= 0,
		Line		= 1,
		Triangle	= 2
	};

	// Buffer Layout
	class GENGINE_API BufferLayout
	{
	public:
		BufferLayout() {}
		BufferLayout(const std::initializer_list<BufferElement>& elements)
			: m_Elements(elements) {
			CalculateOffsetsAndStride();
		}

		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }
		inline uint32_t GetStride() const { return m_Stride; }
		inline uint32_t GetStrideInstance() const { return m_StrideInstance; }
		uint32_t GetElementCount() const { return (uint32_t)m_Elements.size(); }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }
	private:
		void CalculateOffsetsAndStride();
	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
		uint32_t m_StrideInstance = 0;
	};

	class IndexBuffer;

	class GENGINE_API VertexBuffer
	{
	public:
		virtual ~VertexBuffer() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetData(const void* data, uint32_t size) = 0;
		virtual void SetDataInstance(const void* data, uint32_t size) = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) = 0;
		virtual const BufferLayout& GetLayout() const = 0;
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const = 0;
		virtual VertexTopology GetVertexTopologyType() = 0;
		virtual bool IsInstanceRendering() = 0;

		static Ref<VertexBuffer> Create(uint32_t size, uint32_t sizeInstance = 0, VertexTopology type = VertexTopology::Triangle);
		static Ref<VertexBuffer> Create(float* vertices, uint32_t size, uint32_t sizeInstance = 0, VertexTopology type = VertexTopology::Triangle);
	};

	class GENGINE_API IndexBuffer
	{
	public:
		virtual ~IndexBuffer() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual uint32_t GetCount() const = 0;


		static Ref<IndexBuffer> Create(const uint32_t* indices, uint32_t count);
	};
}