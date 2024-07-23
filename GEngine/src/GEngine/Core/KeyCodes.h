#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
// From GE3.h
/* Printable keys */
#define GE_KEY_SPACE              32
#define GE_KEY_APOSTROPHE         39  /* ' */
#define GE_KEY_COMMA              44  /* , */
#define GE_KEY_MINUS              45  /* - */
#define GE_KEY_PERIOD             46  /* . */
#define GE_KEY_SLASH              47  /* / */
#define GE_KEY_0                  48
#define GE_KEY_1                  49
#define GE_KEY_2                  50
#define GE_KEY_3                  51
#define GE_KEY_4                  52
#define GE_KEY_5                  53
#define GE_KEY_6                  54
#define GE_KEY_7                  55
#define GE_KEY_8                  56
#define GE_KEY_9                  57
#define GE_KEY_SEMICOLON          59  /* ; */
#define GE_KEY_EQUAL              61  /* = */
#define GE_KEY_A                  65
#define GE_KEY_B                  66
#define GE_KEY_C                  67
#define GE_KEY_D                  68
#define GE_KEY_E                  69
#define GE_KEY_F                  70
#define GE_KEY_G                  71
#define GE_KEY_H                  72
#define GE_KEY_I                  73
#define GE_KEY_J                  74
#define GE_KEY_K                  75
#define GE_KEY_L                  76
#define GE_KEY_M                  77
#define GE_KEY_N                  78
#define GE_KEY_O                  79
#define GE_KEY_P                  80
#define GE_KEY_Q                  81
#define GE_KEY_R                  82
#define GE_KEY_S                  83
#define GE_KEY_T                  84
#define GE_KEY_U                  85
#define GE_KEY_V                  86
#define GE_KEY_W                  87
#define GE_KEY_X                  88
#define GE_KEY_Y                  89
#define GE_KEY_Z                  90
#define GE_KEY_LEFT_BRACKET       91  /* [ */
#define GE_KEY_BACKSLASH          92  /* \ */
#define GE_KEY_RIGHT_BRACKET      93  /* ] */
#define GE_KEY_GRAVE_ACCENT       96  /* ` */
#define GE_KEY_WORLD_1            161 /* non-US #1 */
#define GE_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define GE_KEY_ESCAPE             256
#define GE_KEY_ENTER              257
#define GE_KEY_TAB                258
#define GE_KEY_BACKSPACE          259
#define GE_KEY_INSERT             260
#define GE_KEY_DELETE             261
#define GE_KEY_RIGHT              262
#define GE_KEY_LEFT               263
#define GE_KEY_DOWN               264
#define GE_KEY_UP                 265
#define GE_KEY_PAGE_UP            266
#define GE_KEY_PAGE_DOWN          267
#define GE_KEY_HOME               268
#define GE_KEY_END                269
#define GE_KEY_CAPS_LOCK          280
#define GE_KEY_SCROLL_LOCK        281
#define GE_KEY_NUM_LOCK           282
#define GE_KEY_PRINT_SCREEN       283
#define GE_KEY_PAUSE              284
#define GE_KEY_F1                 290
#define GE_KEY_F2                 291
#define GE_KEY_F3                 292
#define GE_KEY_F4                 293
#define GE_KEY_F5                 294
#define GE_KEY_F6                 295
#define GE_KEY_F7                 296
#define GE_KEY_F8                 297
#define GE_KEY_F9                 298
#define GE_KEY_F10                299
#define GE_KEY_F11                300
#define GE_KEY_F12                301
#define GE_KEY_F13                302
#define GE_KEY_F14                303
#define GE_KEY_F15                304
#define GE_KEY_F16                305
#define GE_KEY_F17                306
#define GE_KEY_F18                307
#define GE_KEY_F19                308
#define GE_KEY_F20                309
#define GE_KEY_F21                310
#define GE_KEY_F22                311
#define GE_KEY_F23                312
#define GE_KEY_F24                313
#define GE_KEY_F25                314
#define GE_KEY_KP_0               320
#define GE_KEY_KP_1               321
#define GE_KEY_KP_2               322
#define GE_KEY_KP_3               323
#define GE_KEY_KP_4               324
#define GE_KEY_KP_5               325
#define GE_KEY_KP_6               326
#define GE_KEY_KP_7               327
#define GE_KEY_KP_8               328
#define GE_KEY_KP_9               329
#define GE_KEY_KP_DECIMAL         330
#define GE_KEY_KP_DIVIDE          331
#define GE_KEY_KP_MULTIPLY        332
#define GE_KEY_KP_SUBTRACT        333
#define GE_KEY_KP_ADD             334
#define GE_KEY_KP_ENTER           335
#define GE_KEY_KP_EQUAL           336
#define GE_KEY_LEFT_SHIFT         340
#define GE_KEY_LEFT_CONTROL       341
#define GE_KEY_LEFT_ALT           342
#define GE_KEY_LEFT_SUPER         343
#define GE_KEY_RIGHT_SHIFT        344
#define GE_KEY_RIGHT_CONTROL      345
#define GE_KEY_RIGHT_ALT          346
#define GE_KEY_RIGHT_SUPER        347
#define GE_KEY_MENU               348


namespace GEngine
{
	typedef enum KeyCode : int32_t
	{
		Space                 =     GE_KEY_SPACE,
		Apostrophe            =     GE_KEY_APOSTROPHE,
		Comma                 =     GE_KEY_COMMA,
		Minus                 =     GE_KEY_MINUS,
		Period                =     GE_KEY_PERIOD,
		Slash                 =     GE_KEY_SLASH,
		D0                    =     GE_KEY_0,
		D1                    =     GE_KEY_1,		
		D2                    =     GE_KEY_2,
		D3                    =     GE_KEY_3,
		D4                    =     GE_KEY_4,
		D5                    =     GE_KEY_5,
		D6                    =     GE_KEY_6,
		D7                    =     GE_KEY_7,
		D8                    =     GE_KEY_8,
		D9                    =     GE_KEY_9,
		Semicolon             =     GE_KEY_SEMICOLON,
		Equal                 =     GE_KEY_EQUAL,
		A                     =     GE_KEY_A,
		B                     =     GE_KEY_B,
		C                     =     GE_KEY_C,
		D                     =     GE_KEY_D,
		E                     =     GE_KEY_E,
		F                     =     GE_KEY_F,
		G                     =     GE_KEY_G,
		H                     =     GE_KEY_H,
		I                     =     GE_KEY_I,
		J                     =     GE_KEY_J,
		K                     =     GE_KEY_K,
		L                     =     GE_KEY_L,
		M                     =     GE_KEY_M,
		N                     =     GE_KEY_N,
		O                     =     GE_KEY_O,
		P                     =     GE_KEY_P,
		Q                     =     GE_KEY_Q,
		R                     =     GE_KEY_R,
		S                     =     GE_KEY_S,
		T                     =     GE_KEY_T,
		U                     =     GE_KEY_U,
		V                     =     GE_KEY_V,
		W                     =     GE_KEY_W,
		X                     =     GE_KEY_X,
		Y                     =     GE_KEY_Y,
		Z                     =     GE_KEY_Z,
		LeftBracket           =     GE_KEY_LEFT_BRACKET,
		Backslash             =     GE_KEY_BACKSLASH,
		RightBracket          =     GE_KEY_RIGHT_BRACKET,
		GraveAccent           =     GE_KEY_GRAVE_ACCENT,
		World1                =     GE_KEY_WORLD_1,
		World2                =     GE_KEY_WORLD_2,
		Escape                =     GE_KEY_ESCAPE,
		Enter                 =     GE_KEY_ENTER,
		Tab                   =     GE_KEY_TAB,
		Backspace             =     GE_KEY_BACKSPACE,
		Insert                =     GE_KEY_INSERT,
		Delete                =     GE_KEY_DELETE,
		Right                 =     GE_KEY_RIGHT,
		Left                  =     GE_KEY_LEFT,
		Down                  =     GE_KEY_DOWN,
		Up                    =     GE_KEY_UP,
		PageUp                =     GE_KEY_PAGE_UP,
		PageDown              =     GE_KEY_PAGE_DOWN,
		Home                  =     GE_KEY_HOME,
		End                   =     GE_KEY_END,
		CapsLock              =     GE_KEY_CAPS_LOCK,
		ScrollLock            =     GE_KEY_SCROLL_LOCK,
		NumLock               =     GE_KEY_NUM_LOCK,
		PrintScreen           =     GE_KEY_PRINT_SCREEN,
		Pause                 =     GE_KEY_PAUSE,
		F1                    =     GE_KEY_F1,
		F2                    =     GE_KEY_F2,
		F3                    =     GE_KEY_F3,
		F4                    =     GE_KEY_F4,
		F5                    =     GE_KEY_F5,	
		F6                    =     GE_KEY_F6,
		F7                    =     GE_KEY_F7,	
		F8                    =     GE_KEY_F8,	
		F9                    =     GE_KEY_F9,
		F10                   =     GE_KEY_F10,
		F11                   =     GE_KEY_F11,	
		F12                   =     GE_KEY_F12,
		F13                   =     GE_KEY_F13,
		F14                   =     GE_KEY_F14,
		F15                   =     GE_KEY_F15,
		F16                   =     GE_KEY_F16,
		F17                   =     GE_KEY_F17,
		F18                   =     GE_KEY_F18,
		F19                   =     GE_KEY_F19,
		F20                   =     GE_KEY_F20,
		F21                   =     GE_KEY_F21,
		F22                   =     GE_KEY_F22,
		F23                   =     GE_KEY_F23,
		F24                   =     GE_KEY_F24,
		F25                   =     GE_KEY_F25,
		KP0                   =     GE_KEY_KP_0,
		KP1                   =     GE_KEY_KP_1,
		KP2                   =     GE_KEY_KP_2,
		KP3                   =     GE_KEY_KP_3,
		KP4                   =     GE_KEY_KP_4,
		KP5                   =     GE_KEY_KP_5,
		KP6                   =     GE_KEY_KP_6,
		KP7                   =     GE_KEY_KP_7,
		KP8                   =     GE_KEY_KP_8,
		KP9                   =     GE_KEY_KP_9,
		KPDecimal             =     GE_KEY_KP_DECIMAL,
		KPDivide              =     GE_KEY_KP_DIVIDE,
		KPMultiply            =     GE_KEY_KP_MULTIPLY,
		KPSubtract            =     GE_KEY_KP_SUBTRACT,
		KPAdd                 =     GE_KEY_KP_ADD,
		KPEnter               =     GE_KEY_KP_ENTER,
		KPEqual               =     GE_KEY_KP_EQUAL,
		LeftShift             =     GE_KEY_LEFT_SHIFT,
		LeftControl           =     GE_KEY_LEFT_CONTROL,
		LeftAlt               =     GE_KEY_LEFT_ALT,	
		LeftSuper             =     GE_KEY_LEFT_SUPER,
		RightShift            =     GE_KEY_RIGHT_SHIFT,
		RightControl          =     GE_KEY_RIGHT_CONTROL,
		RightAlt              =     GE_KEY_RIGHT_ALT,
		RightSuper            =     GE_KEY_RIGHT_SUPER,
		Menu                  =     GE_KEY_MENU
	};
	
	static std::unordered_map<uint32_t, std::string> KeyCodeString
	{
		{ Space                 ,     "GE_KEY_SPACE"			},
		{ Apostrophe            ,     "GE_KEY_APOSTROPHE"		},
		{ Comma                 ,     "GE_KEY_COMMA"			},
		{ Minus                 ,     "GE_KEY_MINUS"			},
		{ Period                ,     "GE_KEY_PERIOD"			},
		{ Slash                 ,     "GE_KEY_SLASH"			},
		{ D0                    ,     "GE_KEY_0"				},
		{ D1                    ,     "GE_KEY_1"				},
		{ D2                    ,     "GE_KEY_2"				},
		{ D3                    ,     "GE_KEY_3"				},
		{ D4                    ,     "GE_KEY_4"				},
		{ D5                    ,     "GE_KEY_5"				},
		{ D6                    ,     "GE_KEY_6"				},
		{ D7                    ,     "GE_KEY_7"				},
		{ D8                    ,     "GE_KEY_8"				},
		{ D9                    ,     "GE_KEY_9"				},
		{ Semicolon             ,     "GE_KEY_SEMICOLON"		},
		{ Equal                 ,     "GE_KEY_EQUAL"			},
		{ A                     ,     "GE_KEY_A"				},
		{ B                     ,     "GE_KEY_B"				},
		{ C                     ,     "GE_KEY_C"				},
		{ D                     ,     "GE_KEY_D"				},
		{ E                     ,     "GE_KEY_E"				},
		{ F                     ,     "GE_KEY_F"				},
		{ G                     ,     "GE_KEY_G"				},
		{ H                     ,     "GE_KEY_H"				},
		{ I                     ,     "GE_KEY_I"				},
		{ J                     ,     "GE_KEY_J"				},
		{ K                     ,     "GE_KEY_K"				},
		{ L                     ,     "GE_KEY_L"				},
		{ M                     ,     "GE_KEY_M"				},
		{ N                     ,     "GE_KEY_N"				},
		{ O                     ,     "GE_KEY_O"				},
		{ P                     ,     "GE_KEY_P"				},
		{ Q                     ,     "GE_KEY_Q"				},
		{ R                     ,     "GE_KEY_R"				},
		{ S                     ,     "GE_KEY_S"				},
		{ T                     ,     "GE_KEY_T"				},
		{ U                     ,     "GE_KEY_U"				},
		{ V                     ,     "GE_KEY_V"				},
		{ W                     ,     "GE_KEY_W"				},
		{ X                     ,     "GE_KEY_X"				},
		{ Y                     ,     "GE_KEY_Y"				},
		{ Z                     ,     "GE_KEY_Z"				},
		{ LeftBracket           ,     "GE_KEY_LEFT_BRACKET"		},
		{ Backslash             ,     "GE_KEY_BACKSLASH"		},
		{ RightBracket          ,     "GE_KEY_RIGHT_BRACKET"	},
		{ GraveAccent           ,     "GE_KEY_GRAVE_ACCENT"		},
		{ World1                ,     "GE_KEY_WORLD_1"			},
		{ World2                ,     "GE_KEY_WORLD_2"			},
		{ Escape                ,     "GE_KEY_ESCAPE"			},
		{ Enter                 ,     "GE_KEY_ENTER"			},
		{ Tab                   ,     "GE_KEY_TAB"				},
		{ Backspace             ,     "GE_KEY_BACKSPACE"		},
		{ Insert                ,     "GE_KEY_INSERT"			},
		{ Delete                ,     "GE_KEY_DELETE"			},
		{ Right                 ,     "GE_KEY_RIGHT"			},
		{ Left                  ,     "GE_KEY_LEFT"				},
		{ Down                  ,     "GE_KEY_DOWN"				},
		{ Up                    ,     "GE_KEY_UP"				},
		{ PageUp                ,     "GE_KEY_PAGE_UP"			},
		{ PageDown              ,     "GE_KEY_PAGE_DOWN"		},
		{ Home                  ,     "GE_KEY_HOME"				},
		{ End                   ,     "GE_KEY_END"				},
		{ CapsLock              ,     "GE_KEY_CAPS_LOCK"		},
		{ ScrollLock            ,     "GE_KEY_SCROLL_LOCK"		},
		{ NumLock               ,     "GE_KEY_NUM_LOCK"			},
		{ PrintScreen           ,     "GE_KEY_PRINT_SCREEN"		},
		{ Pause                 ,     "GE_KEY_PAUSE"			},
		{ F1                    ,     "GE_KEY_F1"				},
		{ F2                    ,     "GE_KEY_F2"				},
		{ F3                    ,     "GE_KEY_F3"				},
		{ F4                    ,     "GE_KEY_F4"				},
		{ F5                    ,     "GE_KEY_F5"				},
		{ F6                    ,     "GE_KEY_F6"				},
		{ F7                    ,     "GE_KEY_F7"				},
		{ F8                    ,     "GE_KEY_F8"				},
		{ F9                    ,     "GE_KEY_F9"				},
		{ F10                   ,     "GE_KEY_F10"				},
		{ F11                   ,     "GE_KEY_F11"				},
		{ F12                   ,     "GE_KEY_F12"				},
		{ F13                   ,     "GE_KEY_F13"				},
		{ F14                   ,     "GE_KEY_F14"				},
		{ F15                   ,     "GE_KEY_F15"				},
		{ F16                   ,     "GE_KEY_F16"				},
		{ F17                   ,     "GE_KEY_F17"				},
		{ F18                   ,     "GE_KEY_F18"				},
		{ F19                   ,     "GE_KEY_F19"				},
		{ F20                   ,     "GE_KEY_F20"				},
		{ F21                   ,     "GE_KEY_F21"				},
		{ F22                   ,     "GE_KEY_F22"				},
		{ F23                   ,     "GE_KEY_F23"				},
		{ F24                   ,     "GE_KEY_F24"				},
		{ F25                   ,     "GE_KEY_F25"				},
		{ KP0                   ,     "GE_KEY_KP_0"				},
		{ KP1                   ,     "GE_KEY_KP_1"				},
		{ KP2                   ,     "GE_KEY_KP_2"				},
		{ KP3                   ,     "GE_KEY_KP_3"				},
		{ KP4                   ,     "GE_KEY_KP_4"				},
		{ KP5                   ,     "GE_KEY_KP_5"				},
		{ KP6                   ,     "GE_KEY_KP_6"				},
		{ KP7                   ,     "GE_KEY_KP_7"				},
		{ KP8                   ,     "GE_KEY_KP_8"				},
		{ KP9                   ,     "GE_KEY_KP_9"				},
		{ KPDecimal             ,     "GE_KEY_KP_DECIMAL"		},
		{ KPDivide              ,     "GE_KEY_KP_DIVIDE"		},
		{ KPMultiply            ,     "GE_KEY_KP_MULTIPLY"		},
		{ KPSubtract            ,     "GE_KEY_KP_SUBTRACT"		},
		{ KPAdd                 ,     "GE_KEY_KP_ADD"			},
		{ KPEnter               ,     "GE_KEY_KP_ENTER"			},
		{ KPEqual               ,     "GE_KEY_KP_EQUAL"			},
		{ LeftShift             ,     "GE_KEY_LEFT_SHIFT"		},
		{ LeftControl           ,     "GE_KEY_LEFT_CONTROL"		},
		{ LeftAlt               ,     "GE_KEY_LEFT_ALT"			},
		{ LeftSuper             ,     "GE_KEY_LEFT_SUPER"		},
		{ RightShift            ,     "GE_KEY_RIGHT_SHIFT"		},
		{ RightControl          ,     "GE_KEY_RIGHT_CONTROL"	},
		{ RightAlt              ,     "GE_KEY_RIGHT_ALT"		},
		{ RightSuper            ,     "GE_KEY_RIGHT_SUPER"		},
		{ Menu                  ,     "GE_KEY_MENU"				}
	};

	inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
	{
		os << KeyCodeString[static_cast<int32_t>(keyCode)];
		return os;
	}
	
}