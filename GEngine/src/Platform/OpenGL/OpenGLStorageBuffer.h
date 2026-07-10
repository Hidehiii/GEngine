#pragma once
#include "GEngine/Compute/StorageBuffer.h"

namespace GEngine
{
	class GENGINE_API OpenGLStorageBuffer : public StorageBuffer
	{
	public:
		OpenGLStorageBuffer(uint32_t size);

		virtual ~OpenGLStorageBuffer();
		virtual void SetData(uint32_t size, const void* data, uint32_t offset = 0) override;
		virtual void Bind(uint32_t slot) override;

		bool operator==(const OpenGLStorageBuffer& other) const
		{
			return m_Buffer == other.m_Buffer;
		}
	private:
		uint32_t		m_Buffer;
	};
}


