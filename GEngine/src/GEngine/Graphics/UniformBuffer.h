#pragma once

#include "GEngine/Core/Core.h"

namespace GEngine
{
	class GENGINE_API UniformBuffer
	{
	public:
		virtual ~UniformBuffer() {}
		virtual void SetData(const void* data, uint32_t size) = 0;

		virtual bool IsDynamic() const { return m_TotalSize > 0; }
		// only for dynamic UBO
		virtual uint32_t GetAlignment() const { return m_Aligment; }
		virtual uint32_t GetOffset() const { return m_Offset; }
		virtual uint32_t GetTotalSize() const { return m_TotalSize; }

		static Ref<UniformBuffer> Create(uint32_t size, uint32_t count = 1); // count only for dynamic UBO

		
	protected:
		uint32_t	m_Aligment = 0;
		uint32_t	m_Offset = 0;
		uint32_t	m_TotalSize = 0; // only for dynamic UBO
	};
}



