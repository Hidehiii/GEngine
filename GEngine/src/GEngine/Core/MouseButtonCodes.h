#pragma once
#include <iostream>
#include <string>
#include <unordered_map>


namespace GEngine
{
	typedef enum MouseCode : uint32_t
	{
		MOUSE_BUTTON_0,
		MOUSE_BUTTON_1,
		MOUSE_BUTTON_2,
		MOUSE_BUTTON_3,
		MOUSE_BUTTON_4,
		MOUSE_BUTTON_5,
		MOUSE_BUTTON_6,
		MOUSE_BUTTON_7,
		MOUSE_BUTTON_LAST = MOUSE_BUTTON_7,
		MOUSE_BUTTON_LEFT = MOUSE_BUTTON_0,
		MOUSE_BUTTON_RIGHT = MOUSE_BUTTON_1,
		MOUSE_BUTTON_MIDDLE = MOUSE_BUTTON_2
	};

	static std::unordered_map<uint32_t, std::string> MouseCodeString
	{
		{ MOUSE_BUTTON_0       ,         "MOUSE_BUTTON_0"	   },
		{ MOUSE_BUTTON_1       ,         "MOUSE_BUTTON_1"	   },
		{ MOUSE_BUTTON_2       ,         "MOUSE_BUTTON_2"	   },
		{ MOUSE_BUTTON_3       ,         "MOUSE_BUTTON_3"	   },
		{ MOUSE_BUTTON_4       ,         "MOUSE_BUTTON_4"	   },
		{ MOUSE_BUTTON_5       ,         "MOUSE_BUTTON_5"	   },
		{ MOUSE_BUTTON_6       ,         "MOUSE_BUTTON_6"	   },
		{ MOUSE_BUTTON_7       ,         "MOUSE_BUTTON_7"	   },
		{ MOUSE_BUTTON_LAST    ,         "MOUSE_BUTTON_LAST"	   },
		{ MOUSE_BUTTON_LEFT    ,         "MOUSE_BUTTON_LEFT"	   },
		{ MOUSE_BUTTON_RIGHT   ,         "MOUSE_BUTTON_RIGHT"   },
		{ MOUSE_BUTTON_MIDDLE  ,         "MOUSE_BUTTON_MIDDLE"  }
	};

	inline std::ostream& operator<<(std::ostream& os, MouseCode mouseCode)
	{
		os << MouseCodeString[static_cast<int32_t>(mouseCode)];
		return os;
	}

	// From glfw3.h
#define GLFW_MOUSE_BUTTON_1         0
#define GLFW_MOUSE_BUTTON_2         1
#define GLFW_MOUSE_BUTTON_3         2
#define GLFW_MOUSE_BUTTON_4         3
#define GLFW_MOUSE_BUTTON_5         4
#define GLFW_MOUSE_BUTTON_6         5
#define GLFW_MOUSE_BUTTON_7         6
#define GLFW_MOUSE_BUTTON_8         7
#define GLFW_MOUSE_BUTTON_LAST      GLFW_MOUSE_BUTTON_8
#define GLFW_MOUSE_BUTTON_LEFT      GLFW_MOUSE_BUTTON_1
#define GLFW_MOUSE_BUTTON_RIGHT     GLFW_MOUSE_BUTTON_2
#define GLFW_MOUSE_BUTTON_MIDDLE    GLFW_MOUSE_BUTTON_3

	static std::unordered_map<MouseCode, uint32_t> MouseCodeToGLFWValue
	{
		{ MOUSE_BUTTON_0,       GLFW_MOUSE_BUTTON_1 },
		{ MOUSE_BUTTON_1,       GLFW_MOUSE_BUTTON_2 },
		{ MOUSE_BUTTON_2,       GLFW_MOUSE_BUTTON_3 },
		{ MOUSE_BUTTON_3,       GLFW_MOUSE_BUTTON_4 },
		{ MOUSE_BUTTON_4,       GLFW_MOUSE_BUTTON_5 },
		{ MOUSE_BUTTON_5,       GLFW_MOUSE_BUTTON_6 },
		{ MOUSE_BUTTON_6,       GLFW_MOUSE_BUTTON_7 },
		{ MOUSE_BUTTON_7,       GLFW_MOUSE_BUTTON_8 },
		{ MOUSE_BUTTON_LAST,    GLFW_MOUSE_BUTTON_LAST },
		{ MOUSE_BUTTON_LEFT,    GLFW_MOUSE_BUTTON_LEFT },
		{ MOUSE_BUTTON_RIGHT,   GLFW_MOUSE_BUTTON_RIGHT },
		{ MOUSE_BUTTON_MIDDLE,  GLFW_MOUSE_BUTTON_MIDDLE }
	};

	inline uint32_t MouseCodeToGLFW(MouseCode mouseCode)
	{
		return MouseCodeToGLFWValue[mouseCode];
	}

	static std::unordered_map<uint32_t, MouseCode> GLFWValueToMouseCode
	{
		{ GLFW_MOUSE_BUTTON_1,       MOUSE_BUTTON_0 },
		{ GLFW_MOUSE_BUTTON_2,       MOUSE_BUTTON_1 },
		{ GLFW_MOUSE_BUTTON_3,       MOUSE_BUTTON_2 },
		{ GLFW_MOUSE_BUTTON_4,       MOUSE_BUTTON_3 },
		{ GLFW_MOUSE_BUTTON_5,       MOUSE_BUTTON_4 },
		{ GLFW_MOUSE_BUTTON_6,       MOUSE_BUTTON_5 },
		{ GLFW_MOUSE_BUTTON_7,       MOUSE_BUTTON_6 },
		{ GLFW_MOUSE_BUTTON_8,       MOUSE_BUTTON_7 },
		{ GLFW_MOUSE_BUTTON_LAST,    MOUSE_BUTTON_LAST },
		{ GLFW_MOUSE_BUTTON_LEFT,    MOUSE_BUTTON_LEFT },
		{ GLFW_MOUSE_BUTTON_RIGHT,   MOUSE_BUTTON_RIGHT },
		{ GLFW_MOUSE_BUTTON_MIDDLE,  MOUSE_BUTTON_MIDDLE }
	};

	inline MouseCode GLFWToMouseCode(uint32_t glfwMouseCode)
	{
		return GLFWValueToMouseCode[glfwMouseCode];
	}
}