#pragma once

#include "GEngine/Core/Core.h"



namespace GEngine
{
	// Events in GEngine are currently blocking, meaning when an event occurs it immediately gets dispatched and must be dealt with right then and there.
	// For the future, a better strategy might be to buffer events in an event bus and process them during the "event" part of the update stage.

	enum EventType
	{
		EVENT_TYPE_NONE,

		EVENT_TYPE_WINDOW_CLOSE,
		EVENT_TYPE_WINDOW_RESIZE,
		EVENT_TYPE_WINDOW_FOCUS,
		EVENT_TYPE_WINDOW_LOST_FOCUS,
		EVENT_TYPE_WINDOW_MOVED,

		EVENT_TYPE_APP_TICK,
		EVENT_TYPE_APP_UPDATE,
		EVENT_TYPE_APP_RENDER,

		EVENT_TYPE_KEY_PRESSED,
		EVNET_TYPE_KEY_LONG_PRESSED,
		EVENT_TYPE_KEY_LONG_DOWN,
		EVENT_TYPE_KEY_LONG_UP,
		EVENT_TYPE_KEY_UP,
		EVENT_TYPE_KEY_DOWN,
		EVENT_TYPE_KEY_CLICK,
		EVENT_TYPE_KEY_TYPED,

		EVENT_TYPE_MOUSE_BUTTON_PRESSED,
		EVENT_TYPE_MOUSE_BUTTON_LONG_PRESSED,
		EVENT_TYPE_MOUSE_BUTTON_LONG_DOWN,
		EVENT_TYPE_MOUSE_BUTTON_LONG_UP,
		EVENT_TYPE_MOUSE_BUTTON_UP,
		EVENT_TYPE_MOUSE_BUTTON_DOWN,
		EVENT_TYPE_MOUSE_BUTTON_CLICK,
		EVENT_TYPE_MOUSE_MOVED,
		EVENT_TYPE_MOUSE_SCROLLED
	};

	enum EventCategory
	{
		EVENT_CATEGORY_NONE = 0,
		EVENT_CATEGORY_APPLICATION = BIT(0),
		EVENT_CATEGORY_INPUT = BIT(1),
		EVENT_CATEGORY_KEYBOARD = BIT(2),
		EVENT_CATEGORY_MOUSE = BIT(3),
		EVENT_CATEGORY_MOUSE_BUTTON = BIT(4)
	};

#define DECLARE_EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::##type; }\
							   virtual EventType GetEventType() const override { return GetStaticType(); }\
							   virtual const char* GetName() const override { return #type; }

#define DECLARE_EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	class GENGINE_API Event
	{
		friend class EventDispatcher;
	public:
		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}

		inline bool IsHandled() const { return Handled; }
		operator bool() const { return this != nullptr; }
	public:
		bool Handled = false;
	};

	class EventDispatcher
	{
	public:

		template<typename T, typename F>
		static bool Dispatch(Event& event, const F& func)
		{
			if (event.GetEventType() == T::GetStaticType())
			{
				event.Handled = func(static_cast<T&>(event));
				return true;
			}
			return false;
		}
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
}