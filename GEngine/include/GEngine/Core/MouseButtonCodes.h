#pragma once
#include <iostream>
#include <string>
#include <unordered_map>


namespace GEngine
{
	typedef enum MouseCode : uint32_t
	{
		MOUSE_BUTTON_UNKNOWN,
		MOUSE_BUTTON_0,
		MOUSE_BUTTON_1,
		MOUSE_BUTTON_2,
		MOUSE_BUTTON_3,
		MOUSE_BUTTON_4,
		MOUSE_BUTTON_5,
		MOUSE_BUTTON_6,
		MOUSE_BUTTON_7,
		MOUSE_BUTTON_LAST	= MOUSE_BUTTON_7,
		MOUSE_BUTTON_LEFT	= MOUSE_BUTTON_0,
		MOUSE_BUTTON_RIGHT	= MOUSE_BUTTON_1,
		MOUSE_BUTTON_MIDDLE = MOUSE_BUTTON_2
	};

	static std::unordered_map<uint32_t, std::string> MouseCodeString
	{
		{ MOUSE_BUTTON_UNKNOWN ,         "MOUSE_BUTTON_UNKNOWN"	},
		{ MOUSE_BUTTON_0       ,         "MOUSE_BUTTON_0"		},
		{ MOUSE_BUTTON_1       ,         "MOUSE_BUTTON_1"		},
		{ MOUSE_BUTTON_2       ,         "MOUSE_BUTTON_2"		},
		{ MOUSE_BUTTON_3       ,         "MOUSE_BUTTON_3"		},
		{ MOUSE_BUTTON_4       ,         "MOUSE_BUTTON_4"		},
		{ MOUSE_BUTTON_5       ,         "MOUSE_BUTTON_5"		},
		{ MOUSE_BUTTON_6       ,         "MOUSE_BUTTON_6"		},
		{ MOUSE_BUTTON_7       ,         "MOUSE_BUTTON_7"		},
		{ MOUSE_BUTTON_LAST    ,         "MOUSE_BUTTON_LAST"	},
		{ MOUSE_BUTTON_LEFT    ,         "MOUSE_BUTTON_LEFT"	},
		{ MOUSE_BUTTON_RIGHT   ,         "MOUSE_BUTTON_RIGHT"   },
		{ MOUSE_BUTTON_MIDDLE  ,         "MOUSE_BUTTON_MIDDLE"  }
	};

	inline std::ostream& operator<<(std::ostream& os, MouseCode mouseCode)
	{
		os << MouseCodeString[static_cast<int32_t>(mouseCode)];
		return os;
	}

}