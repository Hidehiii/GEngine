#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsCommon.h"

namespace GEngine
{
	class GENGINE_API ShaderInputBufferData
	{
	public:
		std::string			Name;
		ShaderInputDataType	Type;
		uint32_t			Size;
		uint32_t			Offset;
		bool				Normalized = false;
		bool				IsInstance = false;
		ShaderInputBufferData() {}
		ShaderInputBufferData(ShaderInputDataType type, const std::string& name, bool isInstance = false, bool normalized = false)
			: Name(name), Type(type), Size(ShaderIuputDataSize()), Offset(0), IsInstance(isInstance), Normalized(normalized) {
		}

		uint32_t ShaderIuputDataSize() const
		{
			switch (Type) {
			case SHADER_INPUT_DATA_TYPE_FLOAT1: return 4;
			case SHADER_INPUT_DATA_TYPE_FLOAT2: return 4 * 2;
			case SHADER_INPUT_DATA_TYPE_FLOAT3: return 4 * 3;
			case SHADER_INPUT_DATA_TYPE_FLOAT4: return 4 * 4;
			case SHADER_INPUT_DATA_TYPE_INT1:   return 4;
			case SHADER_INPUT_DATA_TYPE_INT2:   return 4 * 2;
			case SHADER_INPUT_DATA_TYPE_INT3:   return 4 * 3;
			case SHADER_INPUT_DATA_TYPE_INT4:   return 4 * 4;
			}
			GE_CORE_ASSERT(false, "Unknown ShaderInputDataType!");
			return 0;
		}
	};

	// Buffer Layout
	class GENGINE_API ShaderInputBufferLayout
	{
	public:
		ShaderInputBufferLayout() {}
		ShaderInputBufferLayout(const std::initializer_list<ShaderInputBufferData>& datas)
			: m_Datas(datas) {
			CalculateOffsetsAndStride();
		}

		inline const std::vector<ShaderInputBufferData>&	GetDatas() const { return m_Datas; }
		inline uint32_t										GetStride() const { return m_Stride; }
		inline uint32_t										GetStrideInstance() const { return m_StrideInstance; }
		inline uint32_t										GetDataCount() const { return (uint32_t)m_Datas.size(); }

		std::vector<ShaderInputBufferData>::iterator begin() { return m_Datas.begin(); }
		std::vector<ShaderInputBufferData>::iterator end() { return m_Datas.end(); }

		std::vector<ShaderInputBufferData>::const_iterator begin() const { return m_Datas.begin(); }
		std::vector<ShaderInputBufferData>::const_iterator end() const { return m_Datas.end(); }
	private:
		void CalculateOffsetsAndStride();
	private:
		std::vector<ShaderInputBufferData>	m_Datas;
		uint32_t							m_Stride = 0;
		uint32_t							m_StrideInstance = 0;
	};

	class CommandBuffer;
	class IndexBuffer;

	class GENGINE_API VertexBuffer
	{
	public:
		virtual ~VertexBuffer() {}

		virtual void SetData(const void* data, uint32_t size) = 0;
		virtual void SetDataInstance(const void* data, uint32_t size) = 0;
		virtual void SetLayout(const ShaderInputBufferLayout& layout) = 0;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) = 0;

		virtual const ShaderInputBufferLayout&		GetLayout() const = 0;
		virtual const Ref<IndexBuffer>&				GetIndexBuffer() const = 0;
		virtual VertexTopology						GetVertexTopologyType() = 0;
		virtual bool								IsInstanceRendering() = 0;

		static Ref<VertexBuffer> Create(uint32_t size, uint32_t sizeInstance = 0, VertexTopology type = VERTEX_TOPOLOGY_TRIANGLE);
		static Ref<VertexBuffer> Create(float* vertices, uint32_t size, uint32_t sizeInstance = 0, VertexTopology type = VERTEX_TOPOLOGY_TRIANGLE);
	protected:
		virtual void Bind(CommandBuffer* cmd) const = 0;

	protected:
		VertexTopology							m_TopologyType;
		ShaderInputBufferLayout					m_Layout;
		

		bool									m_InstanceRendering = false;



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