#pragma once


#include "Event.h"




namespace GEngine
{
	class GENGINE_API WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(unsigned int width, unsigned int height)
			: m_Width(width), m_Height(height) {}

		inline unsigned int GetWidth() const { return m_Width; }
		inline unsigned int GetHeight() const { return m_Height; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

		DECLARE_EVENT_CLASS_TYPE(EVENT_TYPE_WINDOW_RESIZE)
		DECLARE_EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APPLICATION)
	private:
		unsigned int m_Width, m_Height;
	};

	class GENGINE_API WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() {}

		DECLARE_EVENT_CLASS_TYPE(EVENT_TYPE_WINDOW_CLOSE)
		DECLARE_EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APPLICATION)
	};

	class GENGINE_API AppTickEvent : public Event
	{
	public:
		AppTickEvent() {}

		DECLARE_EVENT_CLASS_TYPE(EVENT_TYPE_APP_TICK)
		DECLARE_EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APPLICATION)
	};

	class GENGINE_API AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent() {}

		DECLARE_EVENT_CLASS_TYPE(EVENT_TYPE_APP_UPDATE)
		DECLARE_EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APPLICATION)
	};

	class GENGINE_API AppRenderEvent : public Event
	{
	public:
		AppRenderEvent() {}

		DECLARE_EVENT_CLASS_TYPE(EVENT_TYPE_APP_RENDER)
		DECLARE_EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APPLICATION)
	};
}