#pragma once

#include "Event.h"
#include "GEngine/Core/KeyCodes.h"


namespace GEngine
{
	class GENGINE_API KeyEvent : public Event
	{
	public:
		inline KeyCode GetKeyCode() const { return m_KeyCode; }

		DECLARE_EVENT_CLASS_CATEGORY(EVENT_CATEGORY_KEYBOARD | EVENT_CATEGORY_INPUT)
	protected:
		KeyEvent(KeyCode keycode)
			: m_KeyCode(keycode) {}

		KeyCode m_KeyCode;
	};

	class GENGINE_API KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(KeyCode keycode)
			: KeyEvent(keycode) {}


		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << KeyCodeString[m_KeyCode];
			return ss.str();
		}

		DECLARE_EVENT_CLASS_TYPE(EVENT_TYPE_KEY_PRESSED)
	};

	class GENGINE_API KeyLongPressedEvent : public KeyEvent
	{
		public:
		KeyLongPressedEvent(KeyCode keycode)
			: KeyEvent(keycode) {}
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyLongPressedEvent: " << KeyCodeString[m_KeyCode];
			return ss.str();
		}
		DECLARE_EVENT_CLASS_TYPE(EVNET_TYPE_KEY_LONG_PRESSED)
	};

	class GENGINE_API KeyLongUpEvent : public KeyEvent
	{
		public:
		KeyLongUpEvent(KeyCode keycode)
			: KeyEvent(keycode) {}
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyLongUpEvent: " << KeyCodeString[m_KeyCode];
			return ss.str();
		}
		DECLARE_EVENT_CLASS_TYPE(EVENT_TYPE_KEY_LONG_UP)
	};

	class GENGINE_API KeyLongDownEvent : public KeyEvent
	{
		public:
		KeyLongDownEvent(KeyCode keycode)
			: KeyEvent(keycode) {}
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyLongDownEvent: " << KeyCodeString[m_KeyCode];
			return ss.str();
		}
		DECLARE_EVENT_CLASS_TYPE(EVENT_TYPE_KEY_LONG_DOWN)
	};

	class GENGINE_API KeyUpEvent : public KeyEvent
	{
	public:
		KeyUpEvent(KeyCode keycode)
			: KeyEvent(keycode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyUpEvent: " << KeyCodeString[m_KeyCode];
			return ss.str();
		}

		DECLARE_EVENT_CLASS_TYPE(EVENT_TYPE_KEY_UP)
	};

	class GENGINE_API KeyDownEvent : public KeyEvent
	{
		public:
		KeyDownEvent(KeyCode keycode)
			: KeyEvent(keycode) {}
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyDownEvent: " << KeyCodeString[m_KeyCode];
			return ss.str();
		}
		DECLARE_EVENT_CLASS_TYPE(EVENT_TYPE_KEY_DOWN)
	};

	class GENGINE_API KeyClickEvent : public KeyEvent
	{
		public:
		KeyClickEvent(KeyCode keycode)
			: KeyEvent(keycode) {}
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyClickEvent: " << KeyCodeString[m_KeyCode];
			return ss.str();
		}
		DECLARE_EVENT_CLASS_TYPE(EVENT_TYPE_KEY_CLICK)
	};

	class GENGINE_API KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(KeyCode keycode)
			: KeyEvent(keycode) {}

		inline int GetRepeatCount() const { return m_RepeatCount; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << KeyCodeString[m_KeyCode];
			return ss.str();
		}

		DECLARE_EVENT_CLASS_TYPE(EVENT_TYPE_KEY_TYPED)
	private:
		int m_RepeatCount;
	};
}