#pragma once
#include "GEngine/Core/Input.h"

namespace GEngine
{
	class GENGINE_API Win32Input: public PlatformInput
	{
	public:
		inline virtual bool GetKeyPressed(KeyCode keycode) override;
		inline virtual bool GetKeyUp(KeyCode keycode) override;
		inline virtual bool GetKeyDown(KeyCode keycode) override;
		inline virtual bool GetKeyLongPressed(KeyCode keycode) override;
		inline virtual bool GetKeyLongUp(KeyCode keycode) override;
		inline virtual bool GetKeyLongDown(KeyCode keycode) override;

		inline virtual bool GetMouseButtonPressed(MouseCode button) override;
		inline virtual bool GetMouseButtonUp(MouseCode button) override;
		inline virtual bool GetMouseButtonDown(MouseCode button) override;
		inline virtual bool GetMouseButtonLongPressed(MouseCode button) override;
		inline virtual bool GetMouseButtonLongUp(MouseCode button) override;
		inline virtual bool GetMouseButtonLongDown(MouseCode button) override;

		inline virtual Vector2 GetMousePosition() override;
		inline virtual float GetMouseX() override;
		inline virtual float GetMouseY() override;
	};
	
	static std::unordered_map<KeyCode, int32_t> KeyCodeToWin32Value
	{
		{ KEY_SPACE			,	VK_SPACE			},
		{ KEY_APOSTROPHE	,   VK_OEM_7			},
		{ KEY_COMMA			,   VK_OEM_COMMA		},
		{ KEY_MINUS			,   VK_OEM_MINUS		},
		{ KEY_PERIOD		,   VK_OEM_PERIOD		},
		{ KEY_SLASH			,   VK_OEM_2			},
		{ KEY_D0			,   '0'					},
		{ KEY_D1			,   '1'					},
		{ KEY_D2			,   '2'					},
		{ KEY_D3			,   '3'					},
		{ KEY_D4			,   '4'					},
		{ KEY_D5			,   '5'					},
		{ KEY_D6			,   '6'					},
		{ KEY_D7			,   '7'					},
		{ KEY_D8			,   '8'					},
		{ KEY_D9			,   '9'					},
		{ KEY_SEMICOLON		,   VK_OEM_1			},
		{ KEY_EQUAL			,   VK_OEM_PLUS			},
		{ KEY_A				,   'A'					},
		{ KEY_B				,   'B'					},
		{ KEY_C				,   'C'					},
		{ KEY_D				,   'D'					},
		{ KEY_E				,   'E'					},
		{ KEY_F				,   'F'					},
		{ KEY_G				,	'G'					},
		{ KEY_H				,   'H'					},
		{ KEY_I				,   'I'					},
		{ KEY_J				,   'J'					},
		{ KEY_K				,   'K'					},
		{ KEY_L				,   'L'					},
		{ KEY_M				,   'M'					},
		{ KEY_N				,   'N'					},
		{ KEY_O				,   'O'					},
		{ KEY_P				,   'P'					},
		{ KEY_Q				,   'Q'					},
		{ KEY_R				,   'R'					},
		{ KEY_S				,   'S'					},
		{ KEY_T				,   'T'					},
		{ KEY_U				,   'U'					},
		{ KEY_V				,	'V'					},
		{ KEY_W				,   'W'					},
		{ KEY_X				,   'X'					},
		{ KEY_Y				,   'Y'					},
		{ KEY_Z				,   'Z'					},
		{ KEY_LEFT_BRACKET  ,   VK_OEM_4			},
		{ KEY_BACKSLASH		,   VK_OEM_5			},
		{ KEY_RIGHT_BRACKET ,   VK_OEM_6			},
		{ KEY_GRAVE_ACCENT  ,   VK_OEM_3			},
		{ KEY_WORLD_1		,   KEY_UNKNOWN			}, // Windows does not have a specific keycode for these keys
		{ KEY_WORLD_2		,   KEY_UNKNOWN			},
		{ KEY_ESCAPE		,   VK_ESCAPE			},
		{ KEY_ENTER			,	VK_RETURN			},
		{ KEY_TAB			,	VK_TAB				},
		{ KEY_BACKSPACE		,	VK_BACK				},
		{ KEY_INSERT		,	VK_INSERT			},
		{ KEY_DELETE		,	VK_DELETE			},
		{ KEY_RIGHT			,	VK_RIGHT			},
		{ KEY_LEFT			,	VK_LEFT				},
		{ KEY_DOWN			,	VK_DOWN				},
		{ KEY_UP			,	VK_UP				},
		{ KEY_PAGE_UP		,	VK_PRIOR			},
		{ KEY_PAGE_DOWN		,	VK_NEXT				},
		{ KEY_HOME			,	VK_HOME				},
		{ KEY_END			,	VK_END				},
		{ KEY_CAPS_LOCK     ,   VK_CAPITAL			},
		{ KEY_SCROLL_LOCK   ,   VK_SCROLL			},
		{ KEY_NUM_LOCK      ,   VK_NUMLOCK			},
		{ KEY_PRINT_SCREEN  ,   VK_SNAPSHOT			},
		{ KEY_PAUSE         ,   VK_PAUSE			},
		{ KEY_F1            ,   VK_F1				},
		{ KEY_F2            ,   VK_F2				},
		{ KEY_F3            ,   VK_F3				},
		{ KEY_F4            ,   VK_F4				},
		{ KEY_F5            ,   VK_F5				},
		{ KEY_F6            ,   VK_F6				},
		{ KEY_F7            ,   VK_F7				},
		{ KEY_F8            ,   VK_F8				},
		{ KEY_F9            ,   VK_F9				},
		{ KEY_F10           ,   VK_F10				},
		{ KEY_F11           ,   VK_F11				},
		{ KEY_F12           ,   VK_F12				},
		{ KEY_F13			,	VK_F13				},
		{ KEY_F14			,	VK_F14				},
		{ KEY_F15			,	VK_F15				},
		{ KEY_F16			,	VK_F16				},
		{ KEY_F17			,	VK_F17				},
		{ KEY_F18			,	VK_F18				},
		{ KEY_F19			,	VK_F19				},
		{ KEY_F20			,	VK_F20				},
		{ KEY_F21			,	VK_F21				},
		{ KEY_F22			,	VK_F22				},
		{ KEY_F23			,	VK_F23				},
		{ KEY_F24			,	VK_F24				},
		{ KEY_F25			,	KEY_UNKNOWN			},
		{ KEY_KP_0			,	VK_NUMPAD0			},
		{ KEY_KP_1			,	VK_NUMPAD1			},
		{ KEY_KP_2			,	VK_NUMPAD2			},
		{ KEY_KP_3			,	VK_NUMPAD3			},
		{ KEY_KP_4			,	VK_NUMPAD4			},
		{ KEY_KP_5			,	VK_NUMPAD5			},
		{ KEY_KP_6			,	VK_NUMPAD6			},
		{ KEY_KP_7			,	VK_NUMPAD7			},
		{ KEY_KP_8			,	VK_NUMPAD8			},
		{ KEY_KP_9			,	VK_NUMPAD9			},
		{ KEY_KP_DECIMAL    ,   VK_DECIMAL			},
		{ KEY_KP_DIVIDE     ,   VK_DIVIDE			},
		{ KEY_KP_MULTIPLY   ,   VK_MULTIPLY			},
		{ KEY_KP_SUBTRACT   ,   VK_SUBTRACT			},
		{ KEY_KP_ADD        ,   VK_ADD				},
		{ KEY_KP_ENTER      ,   VK_SEPARATOR		},
		{ KEY_KP_EQUAL      ,   VK_OEM_NEC_EQUAL	},
		{ KEY_LEFT_SHIFT	,   VK_LSHIFT			},
		{ KEY_LEFT_CONTROL  ,   VK_LCONTROL			},
		{ KEY_LEFT_ALT      ,   VK_LMENU			},
		{ KEY_LEFT_SUPER    ,   VK_LWIN				},
		{ KEY_RIGHT_SHIFT   ,   VK_RSHIFT			},
		{ KEY_RIGHT_CONTROL ,   VK_RCONTROL			},
		{ KEY_RIGHT_ALT     ,   VK_RMENU			},
		{ KEY_RIGHT_SUPER   ,   VK_RWIN				},
		{ KEY_MENU          ,   VK_APPS				}
	};

	inline int32_t KeyCodeToWin32(KeyCode keycode)
	{
		if (KeyCodeToWin32Value.find(keycode) != KeyCodeToWin32Value.end())
			return KeyCodeToWin32Value[keycode];
		else
			return KEY_UNKNOWN;
	}

	static std::unordered_map<int32_t, KeyCode> Win32ValueToKeyCode
	{
		{ VK_SPACE			,	KEY_SPACE			},
		{ VK_OEM_7			,   KEY_APOSTROPHE		},
		{ VK_OEM_COMMA		,   KEY_COMMA			},
		{ VK_OEM_MINUS		,   KEY_MINUS			},
		{ VK_OEM_PERIOD		,   KEY_PERIOD			},
		{ VK_OEM_2			,   KEY_SLASH			},
		{ '0'				,   KEY_D0				},
		{ '1'				,   KEY_D1				},
		{ '2'				,   KEY_D2				},
		{ '3'				,   KEY_D3				},
		{ '4'				,   KEY_D4				},
		{ '5'				,   KEY_D5				},
		{ '6'				,   KEY_D6				},
		{ '7'				,   KEY_D7				},
		{ '8'				,   KEY_D8				},
		{ '9'				,   KEY_D9				},
		{ VK_OEM_1			,   KEY_SEMICOLON		},
		{ VK_OEM_PLUS		,   KEY_EQUAL			},
		{ 'A'				,   KEY_A				},
		{ 'B'				,   KEY_B				},
		{ 'C'				,   KEY_C				},
		{ 'D'				,   KEY_D				},
		{ 'E'				,   KEY_E				},
		{ 'F'				,   KEY_F				},
		{ 'G'				,   KEY_G				},
		{ 'H'				,   KEY_H				},
		{ 'I'				,   KEY_I				},
		{ 'J'				,   KEY_J				},
		{ 'K'				,   KEY_K				},
		{ 'L'				,   KEY_L				},
		{ 'M'				,   KEY_M				},
		{ 'N'				,   KEY_N				},
		{ 'O'				,   KEY_O				},
		{ 'P'				,   KEY_P				},
		{ 'Q'				,   KEY_Q				},
		{ 'R'				,   KEY_R				},
		{ 'S'				,   KEY_S				},
		{ 'T'				,   KEY_T				},
		{ 'U'				,   KEY_U				},
		{ 'V'				,   KEY_V				},
		{ 'W'				,   KEY_W				},
		{ 'X'				,   KEY_X				},
		{ 'Y'				,   KEY_Y				},
		{ 'Z'				,   KEY_Z				},
		{ VK_OEM_4			,	KEY_LEFT_BRACKET	},
		{ VK_OEM_5			,	KEY_BACKSLASH		},
		{ VK_OEM_6			,	KEY_RIGHT_BRACKET	},
		{ VK_OEM_3			,	KEY_GRAVE_ACCENT	},
		{ VK_ESCAPE			,	KEY_ESCAPE			},
		{ VK_RETURN			,	KEY_ENTER			},
		{ VK_TAB			,	KEY_TAB				},
		{ VK_BACK			,	KEY_BACKSPACE		},
		{ VK_INSERT			,	KEY_INSERT			},
		{ VK_DELETE			,	KEY_DELETE			},
		{ VK_RIGHT			,	KEY_RIGHT			},
		{ VK_LEFT			,	KEY_LEFT			},
		{ VK_DOWN			,	KEY_DOWN			},
		{ VK_UP				,	KEY_UP				},
		{ VK_PRIOR			,	KEY_PAGE_UP			},
		{ VK_NEXT			,	KEY_PAGE_DOWN		},
		{ VK_HOME			,	KEY_HOME			},
		{ VK_END			,	KEY_END				},
		{ VK_CAPITAL		,   KEY_CAPS_LOCK		},
		{ VK_SCROLL			,   KEY_SCROLL_LOCK		},
		{ VK_NUMLOCK		,   KEY_NUM_LOCK		},
		{ VK_SNAPSHOT		,   KEY_PRINT_SCREEN	},
		{ VK_PAUSE			,   KEY_PAUSE			},
		{ VK_F1				,	KEY_F1				},
		{ VK_F2				,	KEY_F2				},
		{ VK_F3				,	KEY_F3				},
		{ VK_F4				,	KEY_F4				},
		{ VK_F5				,	KEY_F5				},
		{ VK_F6				,	KEY_F6				},
		{ VK_F7				,	KEY_F7				},
		{ VK_F8				,	KEY_F8				},
		{ VK_F9				,	KEY_F9				},
		{ VK_F10			,	KEY_F10				},
		{ VK_F11			,	KEY_F11				},
		{ VK_F12			,	KEY_F12				},
		{ VK_F13			,	KEY_F13				},
		{ VK_F14			,	KEY_F14				},
		{ VK_F15			,	KEY_F15				},
		{ VK_F16			,	KEY_F16				},
		{ VK_F17			,	KEY_F17				},
		{ VK_F18			,	KEY_F18				},
		{ VK_F19			,	KEY_F19				},
		{ VK_F20			,	KEY_F20				},
		{ VK_F21			,	KEY_F21				},
		{ VK_F22			,	KEY_F22				},
		{ VK_F23			,	KEY_F23				},
		{ VK_F24			,	KEY_F24				},
		{ VK_NUMPAD0		,	KEY_KP_0			},
		{ VK_NUMPAD1		,	KEY_KP_1			},
		{ VK_NUMPAD2		,	KEY_KP_2			},
		{ VK_NUMPAD3		,	KEY_KP_3			},
		{ VK_NUMPAD4		,	KEY_KP_4			},
		{ VK_NUMPAD5		,	KEY_KP_5			},
		{ VK_NUMPAD6		,	KEY_KP_6			},
		{ VK_NUMPAD7		,	KEY_KP_7			},
		{ VK_NUMPAD8		,	KEY_KP_8			},
		{ VK_NUMPAD9		,	KEY_KP_9			},
		{ VK_DECIMAL		,   KEY_KP_DECIMAL		},
		{ VK_DIVIDE			,   KEY_KP_DIVIDE		},
		{ VK_MULTIPLY		,   KEY_KP_MULTIPLY		},
		{ VK_SUBTRACT		,   KEY_KP_SUBTRACT		},
		{ VK_ADD			,   KEY_KP_ADD			},
		{ VK_SEPARATOR		,   KEY_KP_ENTER		},
		{ VK_OEM_NEC_EQUAL	,   KEY_KP_EQUAL		},
		{ VK_LSHIFT			,   KEY_LEFT_SHIFT		},
		{ VK_LCONTROL		,   KEY_LEFT_CONTROL	},
		{ VK_LMENU			,   KEY_LEFT_ALT		},
		{ VK_LWIN			,   KEY_LEFT_SUPER		},
		{ VK_RSHIFT			,   KEY_RIGHT_SHIFT		},
		{ VK_RCONTROL		,   KEY_RIGHT_CONTROL	},
		{ VK_RMENU			,   KEY_RIGHT_ALT		},
		{ VK_RWIN			,   KEY_RIGHT_SUPER		},
		{ VK_APPS			,   KEY_MENU			}
	};

	inline KeyCode Win32ToKeyCode(int32_t win32Keycode)
	{
		if (Win32ValueToKeyCode.find(win32Keycode) != Win32ValueToKeyCode.end())
			return Win32ValueToKeyCode[win32Keycode];
		else
			return KEY_UNKNOWN;
	}

	static std::unordered_map<MouseCode, uint32_t> MouseCodeToWin32Value
	{
		{ MOUSE_BUTTON_0		,	VK_LBUTTON				},
		{ MOUSE_BUTTON_1		,	VK_RBUTTON				},
		{ MOUSE_BUTTON_2		,	VK_MBUTTON				},
		{ MOUSE_BUTTON_3		,	VK_XBUTTON1				},
		{ MOUSE_BUTTON_4		,	VK_XBUTTON2				},
		{ MOUSE_BUTTON_5		,	MOUSE_BUTTON_UNKNOWN	},
		{ MOUSE_BUTTON_6		,	MOUSE_BUTTON_UNKNOWN	},
		{ MOUSE_BUTTON_7		,	MOUSE_BUTTON_UNKNOWN	},
		{ MOUSE_BUTTON_LAST		,   MOUSE_BUTTON_UNKNOWN	},
		{ MOUSE_BUTTON_LEFT		,   VK_LBUTTON				},
		{ MOUSE_BUTTON_RIGHT	,   VK_RBUTTON				},
		{ MOUSE_BUTTON_MIDDLE	,	VK_MBUTTON				}
	};

	inline uint32_t MouseCodeToWin32(MouseCode mouseCode)
	{
		if (MouseCodeToWin32Value.find(mouseCode) != MouseCodeToWin32Value.end())
			return MouseCodeToWin32Value[mouseCode];
		else
			return MOUSE_BUTTON_UNKNOWN;
	}

	static std::unordered_map<uint32_t, MouseCode> Win32ValueToMouseCode
	{
		{ VK_LBUTTON				,	MOUSE_BUTTON_0			},
		{ VK_RBUTTON				,	MOUSE_BUTTON_1			},
		{ VK_MBUTTON				,	MOUSE_BUTTON_2			},
		{ VK_XBUTTON1				,	MOUSE_BUTTON_3			},
		{ VK_XBUTTON2				,	MOUSE_BUTTON_4			}
	};

	inline MouseCode Win32ToMouseCode(uint32_t win32MouseCode)
	{
		if (Win32ValueToMouseCode.find(win32MouseCode) != Win32ValueToMouseCode.end())
			return Win32ValueToMouseCode[win32MouseCode];
		else
			return MOUSE_BUTTON_UNKNOWN;
	}
}

