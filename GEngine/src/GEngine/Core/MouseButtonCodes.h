#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
// From glfw3.h
#define GE_MOUSE_BUTTON_1         0
#define GE_MOUSE_BUTTON_2         1
#define GE_MOUSE_BUTTON_3         2
#define GE_MOUSE_BUTTON_4         3
#define GE_MOUSE_BUTTON_5         4
#define GE_MOUSE_BUTTON_6         5
#define GE_MOUSE_BUTTON_7         6
#define GE_MOUSE_BUTTON_8         7
#define GE_MOUSE_BUTTON_LAST      GE_MOUSE_BUTTON_8
#define GE_MOUSE_BUTTON_LEFT      GE_MOUSE_BUTTON_1
#define GE_MOUSE_BUTTON_RIGHT     GE_MOUSE_BUTTON_2
#define GE_MOUSE_BUTTON_MIDDLE    GE_MOUSE_BUTTON_3

namespace GEngine
{
	typedef enum MouseCode : uint32_t
	{
		Button0              =         GE_MOUSE_BUTTON_1,
		Button1              =         GE_MOUSE_BUTTON_2,
		Button2              =         GE_MOUSE_BUTTON_3,
		Button3              =         GE_MOUSE_BUTTON_4,
		Button4              =         GE_MOUSE_BUTTON_5,
		Button5              =         GE_MOUSE_BUTTON_6,
		Button6              =         GE_MOUSE_BUTTON_7,
		Button7              =         GE_MOUSE_BUTTON_8,
		ButtonLast           =         GE_MOUSE_BUTTON_LAST,
		ButtonLeft           =         GE_MOUSE_BUTTON_LEFT,
		ButtonRight          =         GE_MOUSE_BUTTON_RIGHT,
		ButtonMiddle         =         GE_MOUSE_BUTTON_MIDDLE
	};

	static std::unordered_map<uint32_t, std::string> MouseCodeString
	{
		{ Button0              ,         "GE_MOUSE_BUTTON_1"	   },
		{ Button1              ,         "GE_MOUSE_BUTTON_2"	   },
		{ Button2              ,         "GE_MOUSE_BUTTON_3"	   },
		{ Button3              ,         "GE_MOUSE_BUTTON_4"	   },
		{ Button4              ,         "GE_MOUSE_BUTTON_5"	   },
		{ Button5              ,         "GE_MOUSE_BUTTON_6"	   },
		{ Button6              ,         "GE_MOUSE_BUTTON_7"	   },
		{ Button7              ,         "GE_MOUSE_BUTTON_8"	   },
		{ ButtonLast           ,         "GE_MOUSE_BUTTON_LAST"	   },
		{ ButtonLeft           ,         "GE_MOUSE_BUTTON_LEFT"	   },
		{ ButtonRight          ,         "GE_MOUSE_BUTTON_RIGHT"   },
		{ ButtonMiddle         ,         "GE_MOUSE_BUTTON_MIDDLE"  }
	};

	inline std::ostream& operator<<(std::ostream& os, MouseCode mouseCode)
	{
		os << MouseCodeString[static_cast<int32_t>(mouseCode)];
		return os;
	}
}