#pragma once
#include "GEngine/Compute/StorageBuffer.h"

namespace GEngine
{
	class GENGINE_API OpenGLStorageBuffer : public StorageBuffer
	{
	public:
		OpenGLStorageBuffer(uint32_t size, uint32_t binding);

		virtual ~OpenGLStorageBuffer();
		virtual void SetData(uint32_t size, const void* data, uint32_t offset = 0) override;
	private:
		uint32_t		m_RendererID;
		uint32_t		m_Binding;
	};
}


