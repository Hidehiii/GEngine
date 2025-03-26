#include "Core.h"

namespace GEngine
{
	class Config
	{
	public:
		Config() {}
		uint8_t		m_GraphicsAPI = 1;
		uint8_t		m_FramesInFlight = 2;
		bool		m_VSync = false;
		uint32_t	m_DynamicUniformCount = 128;
		uint32_t	m_CommandBufferCount = 1000;
	};
}