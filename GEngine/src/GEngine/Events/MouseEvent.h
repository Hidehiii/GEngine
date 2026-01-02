#pragma once

#include "Event.h"
#include  "GEngine/Core/MouseButtonCodes.h"

namespace GEngine
{
	class GENGINE_API MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float x, float y)
			: m_MouseX(x), m_MouseY(y) {}

		inline float GetX() const { return m_MouseX; }
		inline float GetY() const { return m_MouseY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

		DECLARE_EVENT_CLASS_TYPE(EVENT_TYPE_MOUSE_MOVED)
		DECLARE_EVENT_CLASS_CATEGORY(EVENT_CATEGORY_MOUSE | EVENT_CATEGORY_INPUT)
	private:
		float m_MouseX, m_MouseY;
	};

	class GENGINE_API MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float xOffset, float yOffset)
			: m_XOffset(xOffset), m_YOffset(yOffset) {}

		inline float GetXOffset() const { return m_XOffset; }
		inline float GetYOffset() const { return m_YOffset; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << m_XOffset << ", " << m_YOffset;
			return ss.str();
		}

		DECLARE_EVENT_CLASS_TYPE(EVENT_TYPE_MOUSE_SCROLLED)
		DECLARE_EVENT_CLASS_CATEGORY(EVENT_CATEGORY_MOUSE | EVENT_CATEGORY_INPUT)
	private:
		float m_XOffset, m_YOffset;
	};

	class GENGINE_API MouseButtonEvent : public Event
	{
	public:
		inline MouseCode GetMouseButton() const { return m_Button; }

		DECLARE_EVENT_CLASS_CATEGORY(EVENT_CATEGORY_MOUSE | EVENT_CATEGORY_INPUT)
	protected:
		MouseButtonEvent(MouseCode button)
			: m_Button(button) {}

		MouseCode m_Button;
	};

	class GENGINE_API MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(MouseCode button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_Button;
			return ss.str();
		}

		DECLARE_EVENT_CLASS_TYPE(EVENT_TYPE_MOUSE_BUTTON_PRESSED)
	};

	class GENGINE_API MouseButtonLongPressedEvent : public MouseButtonEvent
	{
		public:
		MouseButtonLongPressedEvent(MouseCode button)
			: MouseButtonEvent(button) {}
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonLongPressedEvent: " << m_Button;
			return ss.str();
		}
		DECLARE_EVENT_CLASS_TYPE(EVENT_TYPE_MOUSE_BUTTON_LONG_PRESSED)
	};

	class GENGINE_API MouseButtonLongDownEvent : public MouseButtonEvent
	{
		public:
		MouseButtonLongDownEvent(MouseCode button)
			: MouseButtonEvent(button) {}
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonLongDownEvent: " << m_Button;
			return ss.str();
		}
		DECLARE_EVENT_CLASS_TYPE(EVENT_TYPE_MOUSE_BUTTON_LONG_DOWN)
	};

	class GENGINE_API MouseButtonLongUpEvent : public MouseButtonEvent
	{
		public:
		MouseButtonLongUpEvent(MouseCode button)
			: MouseButtonEvent(button) {}
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonLongUpEvent: " << m_Button;
			return ss.str();
		}
		DECLARE_EVENT_CLASS_TYPE(EVENT_TYPE_MOUSE_BUTTON_LONG_UP)
	};

	class GENGINE_API MouseButtonUpEvent : public MouseButtonEvent
	{
	public:
		MouseButtonUpEvent(MouseCode button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_Button;
			return ss.str();
		}

		DECLARE_EVENT_CLASS_TYPE(EVENT_TYPE_MOUSE_BUTTON_UP)
	};

	class GENGINE_API MouseButtonDownEvent : public MouseButtonEvent
	{
		public:
		MouseButtonDownEvent(MouseCode button)
			: MouseButtonEvent(button) {}
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonDownEvent: " << m_Button;
			return ss.str();
		}
		DECLARE_EVENT_CLASS_TYPE(EVENT_TYPE_MOUSE_BUTTON_DOWN)
	};

	class GENGINE_API MouseButtonClickEvent : public MouseButtonEvent
	{
		public:
		MouseButtonClickEvent(MouseCode button)
			: MouseButtonEvent(button) {}
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonClickEvent: " << m_Button;
			return ss.str();
		}
		DECLARE_EVENT_CLASS_TYPE(EVENT_TYPE_MOUSE_BUTTON_CLICK)
	};
}