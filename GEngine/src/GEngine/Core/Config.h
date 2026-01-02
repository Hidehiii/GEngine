#include "Core.h"

namespace GEngine
{
	class Config
	{
	public:
		Config() {}
		uint8_t			m_GraphicsAPI = 2;
		uint8_t			m_FramesInFlight = 2;
		bool			m_VSync = false;
		uint32_t		m_CommandBufferCount = 1000;
		std::string		m_ShaderCacheDirectory = "Assets/Cache/Shaders/";
		uint32_t	    m_LongPressThresholdMs = 500; // 长按阈值，单位：毫秒
	};
}