#pragma once

#include "GEngine/Core/Core.h"

namespace GEngine
{
	class GENGINE_API UniformBuffer
	{
	public:
		virtual ~UniformBuffer() {}
		virtual void SetData(const void* data, uint32_t size) = 0;
		virtual void SetDataDynamic(const void* data, uint32_t size) = 0;

		virtual bool IsDynamic() const { return m_TotalSize > 0; }
		// only for dynamic UBO
		virtual uint32_t	GetAlignment() const { return m_Aligment; }
		virtual uint32_t	GetOffset() const { return m_Offset; }
		virtual uint32_t	GetTotalSize() const { return m_TotalSize; }
		virtual bool		IsAutoSetDataDynamic() const { return m_AutoSetDataDynamic; }

		// size is the size of a single UBO, 
		// count is only for dynamic UBO, it is the number of UBOs in the dynamic UBO
		// each UBO in the dynamic UBO will be aligned to the minimum uniform buffer offset alignment
		// autoSetDataDynamic is only for dynamic UBO,
		// if true, each time the material update would call SetDataDynamic() automatically
		// if false, the user need to call SetDataDynamic() manually and the material would just call SetData()
		static Ref<UniformBuffer> Create(uint32_t size, uint32_t count = 1, bool autoSetDataDynamic = true);

		
	protected:
		uint32_t	m_Aligment = 0;
		uint32_t	m_Offset = 0;
		uint32_t	m_TotalSize = 0; // only for dynamic UBO
		bool		m_AutoSetDataDynamic = false;
	};
}



