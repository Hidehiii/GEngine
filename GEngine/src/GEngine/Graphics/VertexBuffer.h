#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsCommon.h"

namespace GEngine
{
	namespace Utils
	{
		inline uint32_t ShaderInputDataSize(ShaderInputDataType type)
		{
			switch (type) {
			case SHADER_INPUT_DATA_TYPE_FLOAT1: return 4;
			case SHADER_INPUT_DATA_TYPE_FLOAT2: return 4 * 2;
			case SHADER_INPUT_DATA_TYPE_FLOAT3: return 4 * 3;
			case SHADER_INPUT_DATA_TYPE_FLOAT4: return 4 * 4;
			case SHADER_INPUT_DATA_TYPE_INT1:   return 4;
			case SHADER_INPUT_DATA_TYPE_INT2:   return 4 * 2;
			case SHADER_INPUT_DATA_TYPE_INT3:   return 4 * 3;
			case SHADER_INPUT_DATA_TYPE_INT4:   return 4 * 4;
			case SHADER_INPUT_DATA_TYPE_UINT1:	return 4;
			case SHADER_INPUT_DATA_TYPE_UINT2:	return 4 * 2;
			case SHADER_INPUT_DATA_TYPE_UINT3:	return 4 * 3;
			case SHADER_INPUT_DATA_TYPE_UINT4:	return 4 * 4;
			}
			GE_CORE_ASSERT(false, "Unknown ShaderInputDataType!");
			return 0;
		}
	}


	class CommandBuffer;
	class IndexBuffer;
	class Shader;

	class GENGINE_API VertexBuffer
	{
	public:
		virtual ~VertexBuffer() {}

		virtual void SetVertexData(const void* data, uint32_t size) = 0;
		virtual void SetInstanceData(const void* data, uint32_t size) = 0;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) = 0;

		virtual const Ref<IndexBuffer>&				GetIndexBuffer() const = 0;
		virtual VertexTopology						GetVertexTopologyType() = 0;
		virtual uint32_t							GetTotalSizeVertex() const { return m_TotalSizeVertex; }
		virtual uint32_t							GetTotalSizeInstance() const { return m_TotalSizeInstance; }
		virtual bool								IsInstanceRendering() const;

		virtual uint32_t GetVertexCount() const;
		virtual uint32_t GetIndexCount() const = 0;
		virtual uint32_t GetInstanceCount() const;

		static Ref<VertexBuffer> Create(uint32_t size, uint32_t sizeInstance = 0, VertexTopology type = VERTEX_TOPOLOGY_TRIANGLE);
		static Ref<VertexBuffer> Create(const void* vertices, uint32_t size, uint32_t sizeInstance = 0, VertexTopology type = VERTEX_TOPOLOGY_TRIANGLE);
	protected:
		virtual void Bind(CommandBuffer* cmd) const = 0;

		virtual void SetShaderAndInputLayout(const Ref<Shader>& shader, uint32_t pass) = 0;
		virtual void CheckInputLayout(const Ref<Shader>& shader, uint32_t pass)
		{
			if (shader != m_Shader || pass != m_ShaderPass)
			{
				SetShaderAndInputLayout(shader, pass);
			}
		}
	protected:
		VertexTopology							m_TopologyType = VERTEX_TOPOLOGY_TRIANGLE;

		uint32_t								m_TotalSizeVertex = 0;
		uint32_t								m_TotalSizeInstance = 0;	

		Ref<Shader>								m_Shader = nullptr; // use the shader reflection to define the input layout, this will be compared before render every time
		uint32_t								m_ShaderPass;

		friend class CommandBuffer;
	};

	class GENGINE_API IndexBuffer
	{
	public:
		virtual ~IndexBuffer() {}

		virtual uint32_t GetCount() const { return m_Count; };

		static Ref<IndexBuffer> Create(const uint32_t* indices, uint32_t count);
	protected:
		virtual void Bind(CommandBuffer* cmd) const = 0;

	protected:

		uint32_t m_Count;

		friend class VertexBuffer;
	};
}