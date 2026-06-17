#include "Core.h"

namespace GEngine
{
	class Config
	{
	public:
		enum CONFIG_WINDOW_MANAGER_API
		{
			CONFIG_WINDOW_MANAGER_API_GLFW = 0,
			CONFIG_WINDOW_MANAGER_API_WIN32 = 1
		};

		Config() {}

		uint8_t		GetGraphicsAPI() const { return m_GraphicsAPI; }
		uint8_t		GetFramesInFlight() const { return m_FramesInFlight; }
		bool		GetVSync() const { return m_VSync; }
		uint32_t	GetCommandBufferCount() const { return m_CommandBufferCount; }
		std::string GetShaderCacheDirectory() const { return m_ShaderCacheDirectory; }
		uint32_t	GetLongPressThresholdMs() const { return m_LongPressThresholdMs; }
		uint8_t		GetWindowManagerAPI() const { return m_WindowManagerAPI; }
	protected:
		uint8_t			m_GraphicsAPI = 2;
		uint8_t			m_FramesInFlight = 2;
		bool			m_VSync = false;
		uint32_t		m_CommandBufferCount = 1000;
		std::string		m_ShaderCacheDirectory = "Assets/Cache/Shaders/";
		uint32_t	    m_LongPressThresholdMs = 500; // 长按阈值，单位：毫秒
		uint8_t			m_WindowManagerAPI = 0; 

		friend class Serializer;
	};
}