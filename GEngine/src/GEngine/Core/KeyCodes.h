#pragma once
#include <iostream>
#include <string>
#include <unordered_map>



namespace GEngine
{
	typedef enum KeyCode : int32_t
	{
		KEY_SPACE,
		KEY_APOSTROPHE,
		KEY_COMMA,
		KEY_MINUS,
		KEY_PERIOD,
		KEY_SLASH,
		KEY_D0,
		KEY_D1,
		KEY_D2,
		KEY_D3,
		KEY_D4,
		KEY_D5,
		KEY_D6,
		KEY_D7,
		KEY_D8,
		KEY_D9,
		KEY_SEMICOLON,
		KEY_EQUAL,
		KEY_A,
		KEY_B,
		KEY_C,
		KEY_D,
		KEY_E,
		KEY_F,
		KEY_G,
		KEY_H,
		KEY_I,
		KEY_J,
		KEY_K,
		KEY_L,
		KEY_M,
		KEY_N,
		KEY_O,
		KEY_P,
		KEY_Q,
		KEY_R,
		KEY_S,
		KEY_T,
		KEY_U,
		KEY_V,
		KEY_W,
		KEY_X,
		KEY_Y,
		KEY_Z,
		KEY_LEFT_BRACKET,
		KEY_BACKSLASH,
		KEY_RIGHT_BRACKET,
		KEY_GRAVE_ACCENT,
		KEY_WORLD_1,
		KEY_WORLD_2,
		KEY_ESCAPE,
		KEY_ENTER,
		KEY_TAB,
		KEY_BACKSPACE,
		KEY_INSERT,
		KEY_DELETE,
		KEY_RIGHT,
		KEY_LEFT,
		KEY_DOWN,
		KEY_UP,
		KEY_PAGE_UP,
		KEY_PAGE_DOWN,
		KEY_HOME,
		KEY_END,
		KEY_CAPS_LOCK,
		KEY_SCROLL_LOCK,
		KEY_NUM_LOCK,
		KEY_PRINT_SCREEN,
		KEY_PAUSE,
		KEY_F1,
		KEY_F2,
		KEY_F3,
		KEY_F4,
		KEY_F5,
		KEY_F6,
		KEY_F7,
		KEY_F8,
		KEY_F9,
		KEY_F10,
		KEY_F11,
		KEY_F12,
		KEY_F13,
		KEY_F14,
		KEY_F15,
		KEY_F16,
		KEY_F17,
		KEY_F18,
		KEY_F19,
		KEY_F20,
		KEY_F21,
		KEY_F22,
		KEY_F23,
		KEY_F24,
		KEY_F25,
		KEY_KP_0,
		KEY_KP_1,
		KEY_KP_2,
		KEY_KP_3,
		KEY_KP_4,
		KEY_KP_5,
		KEY_KP_6,
		KEY_KP_7,
		KEY_KP_8,
		KEY_KP_9,
		KEY_KP_DECIMAL,
		KEY_KP_DIVIDE,
		KEY_KP_MULTIPLY,
		KEY_KP_SUBTRACT,
		KEY_KP_ADD,
		KEY_KP_ENTER,
		KEY_KP_EQUAL,
		KEY_LEFT_SHIFT,
		KEY_LEFT_CONTROL,
		KEY_LEFT_ALT,
		KEY_LEFT_SUPER,
		KEY_RIGHT_SHIFT,
		KEY_RIGHT_CONTROL,
		KEY_RIGHT_ALT,
		KEY_RIGHT_SUPER,
		KEY_MENU
	};
	
	static std::unordered_map<uint32_t, std::string> KeyCodeString
	{
		{ KEY_SPACE             ,     "KEY_SPACE"			},
		{ KEY_APOSTROPHE        ,     "KEY_APOSTROPHE"		},
		{ KEY_COMMA             ,     "KEY_COMMA"			},
		{ KEY_MINUS             ,     "KEY_MINUS"			},
		{ KEY_PERIOD            ,     "KEY_PERIOD"			},
		{ KEY_SLASH             ,     "KEY_SLASH"			},
		{ KEY_D0                ,     "KEY_0"				},
		{ KEY_D1                ,     "KEY_1"				},
		{ KEY_D2                ,     "KEY_2"				},
		{ KEY_D3                ,     "KEY_3"				},
		{ KEY_D4                ,     "KEY_4"				},
		{ KEY_D5                ,     "KEY_5"				},
		{ KEY_D6                ,     "KEY_6"				},
		{ KEY_D7                ,     "KEY_7"				},
		{ KEY_D8                ,     "KEY_8"				},
		{ KEY_D9                ,     "KEY_9"				},
		{ KEY_SEMICOLON         ,     "KEY_SEMICOLON"		},
		{ KEY_EQUAL             ,     "KEY_EQUAL"			},
		{ KEY_A                 ,     "KEY_A"				},
		{ KEY_B                 ,     "KEY_B"				},
		{ KEY_C                 ,     "KEY_C"				},
		{ KEY_D                 ,     "KEY_D"				},
		{ KEY_E                 ,     "KEY_E"				},
		{ KEY_F                 ,     "KEY_F"				},
		{ KEY_G                 ,     "KEY_G"				},
		{ KEY_H                 ,     "KEY_H"				},
		{ KEY_I                 ,     "KEY_I"				},
		{ KEY_J                 ,     "KEY_J"				},
		{ KEY_K                 ,     "KEY_K"				},
		{ KEY_L                 ,     "KEY_L"				},
		{ KEY_M                 ,     "KEY_M"				},
		{ KEY_N                 ,     "KEY_N"				},
		{ KEY_O                 ,     "KEY_O"				},
		{ KEY_P                 ,     "KEY_P"				},
		{ KEY_Q                 ,     "KEY_Q"				},
		{ KEY_R                 ,     "KEY_R"				},
		{ KEY_S                 ,     "KEY_S"				},
		{ KEY_T                 ,     "KEY_T"				},
		{ KEY_U                 ,     "KEY_U"				},
		{ KEY_V                 ,     "KEY_V"				},
		{ KEY_W                 ,     "KEY_W"				},
		{ KEY_X                 ,     "KEY_X"				},
		{ KEY_Y                 ,     "KEY_Y"				},
		{ KEY_Z                 ,     "KEY_Z"				},
		{ KEY_LEFT_BRACKET      ,     "KEY_LEFT_BRACKET"		},
		{ KEY_BACKSLASH         ,     "KEY_BACKSLASH"		},
		{ KEY_RIGHT_BRACKET     ,     "KEY_RIGHT_BRACKET"	},
		{ KEY_GRAVE_ACCENT      ,     "KEY_GRAVE_ACCENT"		},
		{ KEY_WORLD_1           ,     "KEY_WORLD_1"			},
		{ KEY_WORLD_2           ,     "KEY_WORLD_2"			},
		{ KEY_ESCAPE            ,     "KEY_ESCAPE"			},
		{ KEY_ENTER             ,     "KEY_ENTER"			},
		{ KEY_TAB               ,     "KEY_TAB"				},
		{ KEY_BACKSPACE         ,     "KEY_BACKSPACE"		},
		{ KEY_INSERT            ,     "KEY_INSERT"			},
		{ KEY_DELETE            ,     "KEY_DELETE"			},
		{ KEY_RIGHT             ,     "KEY_RIGHT"			},
		{ KEY_LEFT              ,     "KEY_LEFT"				},
		{ KEY_DOWN              ,     "KEY_DOWN"				},
		{ KEY_UP                ,     "KEY_UP"				},
		{ KEY_PAGE_UP           ,     "KEY_PAGE_UP"			},
		{ KEY_PAGE_DOWN         ,     "KEY_PAGE_DOWN"		},
		{ KEY_HOME              ,     "KEY_HOME"				},
		{ KEY_END               ,     "KEY_END"				},
		{ KEY_CAPS_LOCK         ,     "KEY_CAPS_LOCK"		},
		{ KEY_SCROLL_LOCK       ,     "KEY_SCROLL_LOCK"		},
		{ KEY_NUM_LOCK          ,     "KEY_NUM_LOCK"			},
		{ KEY_PRINT_SCREEN      ,     "KEY_PRINT_SCREEN"		},
		{ KEY_PAUSE             ,     "KEY_PAUSE"			},
		{ KEY_F1                ,     "KEY_F1"				},
		{ KEY_F2                ,     "KEY_F2"				},
		{ KEY_F3                ,     "KEY_F3"				},
		{ KEY_F4                ,     "KEY_F4"				},
		{ KEY_F5                ,     "KEY_F5"				},
		{ KEY_F6                ,     "KEY_F6"				},
		{ KEY_F7                ,     "KEY_F7"				},
		{ KEY_F8                ,     "KEY_F8"				},
		{ KEY_F9                ,     "KEY_F9"				},
		{ KEY_F10               ,     "KEY_F10"				},
		{ KEY_F11               ,     "KEY_F11"				},
		{ KEY_F12               ,     "KEY_F12"				},
		{ KEY_F13               ,     "KEY_F13"				},
		{ KEY_F14               ,     "KEY_F14"				},
		{ KEY_F15               ,     "KEY_F15"				},
		{ KEY_F16               ,     "KEY_F16"				},
		{ KEY_F17               ,     "KEY_F17"				},
		{ KEY_F18               ,     "KEY_F18"				},
		{ KEY_F19               ,     "KEY_F19"				},
		{ KEY_F20               ,     "KEY_F20"				},
		{ KEY_F21               ,     "KEY_F21"				},
		{ KEY_F22               ,     "KEY_F22"				},
		{ KEY_F23               ,     "KEY_F23"				},
		{ KEY_F24               ,     "KEY_F24"				},
		{ KEY_F25               ,     "KEY_F25"				},
		{ KEY_KP_0              ,     "KEY_KP_0"				},
		{ KEY_KP_1              ,     "KEY_KP_1"				},
		{ KEY_KP_2              ,     "KEY_KP_2"				},
		{ KEY_KP_3              ,     "KEY_KP_3"				},
		{ KEY_KP_4              ,     "KEY_KP_4"				},
		{ KEY_KP_5              ,     "KEY_KP_5"				},
		{ KEY_KP_6              ,     "KEY_KP_6"				},
		{ KEY_KP_7              ,     "KEY_KP_7"				},
		{ KEY_KP_8              ,     "KEY_KP_8"				},
		{ KEY_KP_9              ,     "KEY_KP_9"				},
		{ KEY_KP_DECIMAL        ,     "KEY_KP_DECIMAL"		},
		{ KEY_KP_DIVIDE         ,     "KEY_KP_DIVIDE"		},
		{ KEY_KP_MULTIPLY       ,     "KEY_KP_MULTIPLY"		},
		{ KEY_KP_SUBTRACT       ,     "KEY_KP_SUBTRACT"		},
		{ KEY_KP_ADD            ,     "KEY_KP_ADD"			},
		{ KEY_KP_ENTER          ,     "KEY_KP_ENTER"			},
		{ KEY_KP_EQUAL          ,     "KEY_KP_EQUAL"			},
		{ KEY_LEFT_SHIFT        ,     "KEY_LEFT_SHIFT"		},
		{ KEY_LEFT_CONTROL      ,     "KEY_LEFT_CONTROL"		},
		{ KEY_LEFT_ALT          ,     "KEY_LEFT_ALT"			},
		{ KEY_LEFT_SUPER        ,     "KEY_LEFT_SUPER"		},
		{ KEY_RIGHT_SHIFT       ,     "KEY_RIGHT_SHIFT"		},
		{ KEY_RIGHT_CONTROL     ,     "KEY_RIGHT_CONTROL"	},
		{ KEY_RIGHT_ALT         ,     "KEY_RIGHT_ALT"		},
		{ KEY_RIGHT_SUPER       ,     "KEY_RIGHT_SUPER"		},
		{ KEY_MENU              ,     "KEY_MENU"				}
	};

	inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
	{
		os << KeyCodeString[static_cast<int32_t>(keyCode)];
		return os;
	}

	// From GE3.h
/* Printable keys */
#define GLFW_KEY_SPACE              32
#define GLFW_KEY_APOSTROPHE         39  /* ' */
#define GLFW_KEY_COMMA              44  /* , */
#define GLFW_KEY_MINUS              45  /* - */
#define GLFW_KEY_PERIOD             46  /* . */
#define GLFW_KEY_SLASH              47  /* / */
#define GLFW_KEY_0                  48
#define GLFW_KEY_1                  49
#define GLFW_KEY_2                  50
#define GLFW_KEY_3                  51
#define GLFW_KEY_4                  52
#define GLFW_KEY_5                  53
#define GLFW_KEY_6                  54
#define GLFW_KEY_7                  55
#define GLFW_KEY_8                  56
#define GLFW_KEY_9                  57
#define GLFW_KEY_SEMICOLON          59  /* ; */
#define GLFW_KEY_EQUAL              61  /* = */
#define GLFW_KEY_A                  65
#define GLFW_KEY_B                  66
#define GLFW_KEY_C                  67
#define GLFW_KEY_D                  68
#define GLFW_KEY_E                  69
#define GLFW_KEY_F                  70
#define GLFW_KEY_G                  71
#define GLFW_KEY_H                  72
#define GLFW_KEY_I                  73
#define GLFW_KEY_J                  74
#define GLFW_KEY_K                  75
#define GLFW_KEY_L                  76
#define GLFW_KEY_M                  77
#define GLFW_KEY_N                  78
#define GLFW_KEY_O                  79
#define GLFW_KEY_P                  80
#define GLFW_KEY_Q                  81
#define GLFW_KEY_R                  82
#define GLFW_KEY_S                  83
#define GLFW_KEY_T                  84
#define GLFW_KEY_U                  85
#define GLFW_KEY_V                  86
#define GLFW_KEY_W                  87
#define GLFW_KEY_X                  88
#define GLFW_KEY_Y                  89
#define GLFW_KEY_Z                  90
#define GLFW_KEY_LEFT_BRACKET       91  /* [ */
#define GLFW_KEY_BACKSLASH          92  /* \ */
#define GLFW_KEY_RIGHT_BRACKET      93  /* ] */
#define GLFW_KEY_GRAVE_ACCENT       96  /* ` */
#define GLFW_KEY_WORLD_1            161 /* non-US #1 */
#define GLFW_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define GLFW_KEY_ESCAPE             256
#define GLFW_KEY_ENTER              257
#define GLFW_KEY_TAB                258
#define GLFW_KEY_BACKSPACE          259
#define GLFW_KEY_INSERT             260
#define GLFW_KEY_DELETE             261
#define GLFW_KEY_RIGHT              262
#define GLFW_KEY_LEFT               263
#define GLFW_KEY_DOWN               264
#define GLFW_KEY_UP                 265
#define GLFW_KEY_PAGE_UP            266
#define GLFW_KEY_PAGE_DOWN          267
#define GLFW_KEY_HOME               268
#define GLFW_KEY_END                269
#define GLFW_KEY_CAPS_LOCK          280
#define GLFW_KEY_SCROLL_LOCK        281
#define GLFW_KEY_NUM_LOCK           282
#define GLFW_KEY_PRINT_SCREEN       283
#define GLFW_KEY_PAUSE              284
#define GLFW_KEY_F1                 290
#define GLFW_KEY_F2                 291
#define GLFW_KEY_F3                 292
#define GLFW_KEY_F4                 293
#define GLFW_KEY_F5                 294
#define GLFW_KEY_F6                 295
#define GLFW_KEY_F7                 296
#define GLFW_KEY_F8                 297
#define GLFW_KEY_F9                 298
#define GLFW_KEY_F10                299
#define GLFW_KEY_F11                300
#define GLFW_KEY_F12                301
#define GLFW_KEY_F13                302
#define GLFW_KEY_F14                303
#define GLFW_KEY_F15                304
#define GLFW_KEY_F16                305
#define GLFW_KEY_F17                306
#define GLFW_KEY_F18                307
#define GLFW_KEY_F19                308
#define GLFW_KEY_F20                309
#define GLFW_KEY_F21                310
#define GLFW_KEY_F22                311
#define GLFW_KEY_F23                312
#define GLFW_KEY_F24                313
#define GLFW_KEY_F25                314
#define GLFW_KEY_KP_0               320
#define GLFW_KEY_KP_1               321
#define GLFW_KEY_KP_2               322
#define GLFW_KEY_KP_3               323
#define GLFW_KEY_KP_4               324
#define GLFW_KEY_KP_5               325
#define GLFW_KEY_KP_6               326
#define GLFW_KEY_KP_7               327
#define GLFW_KEY_KP_8               328
#define GLFW_KEY_KP_9               329
#define GLFW_KEY_KP_DECIMAL         330
#define GLFW_KEY_KP_DIVIDE          331
#define GLFW_KEY_KP_MULTIPLY        332
#define GLFW_KEY_KP_SUBTRACT        333
#define GLFW_KEY_KP_ADD             334
#define GLFW_KEY_KP_ENTER           335
#define GLFW_KEY_KP_EQUAL           336
#define GLFW_KEY_LEFT_SHIFT         340
#define GLFW_KEY_LEFT_CONTROL       341
#define GLFW_KEY_LEFT_ALT           342
#define GLFW_KEY_LEFT_SUPER         343
#define GLFW_KEY_RIGHT_SHIFT        344
#define GLFW_KEY_RIGHT_CONTROL      345
#define GLFW_KEY_RIGHT_ALT          346
#define GLFW_KEY_RIGHT_SUPER        347
#define GLFW_KEY_MENU               348

	static std::unordered_map<KeyCode, int32_t> KeyCodeToGLFWValue
	{
		{ KEY_SPACE             ,     GLFW_KEY_SPACE             },
		{ KEY_APOSTROPHE        ,     GLFW_KEY_APOSTROPHE        },
		{ KEY_COMMA             ,     GLFW_KEY_COMMA             },
		{ KEY_MINUS             ,     GLFW_KEY_MINUS             },
		{ KEY_PERIOD            ,     GLFW_KEY_PERIOD            },
		{ KEY_SLASH             ,     GLFW_KEY_SLASH             },
		{ KEY_D0                ,     GLFW_KEY_0                 },
		{ KEY_D1                ,     GLFW_KEY_1                 },
		{ KEY_D2                ,     GLFW_KEY_2                 },
		{ KEY_D3                ,     GLFW_KEY_3                 },
		{ KEY_D4                ,     GLFW_KEY_4                 },
		{ KEY_D5                ,     GLFW_KEY_5                 },
		{ KEY_D6                ,     GLFW_KEY_6                 },
		{ KEY_D7                ,     GLFW_KEY_7                 },
		{ KEY_D8                ,     GLFW_KEY_8                 },
		{ KEY_D9                ,     GLFW_KEY_9                 },
		{ KEY_SEMICOLON         ,     GLFW_KEY_SEMICOLON         },
		{ KEY_EQUAL             ,     GLFW_KEY_EQUAL             },
		{ KEY_A                 ,     GLFW_KEY_A                 },
		{ KEY_B                 ,     GLFW_KEY_B                 },
		{ KEY_C                 ,     GLFW_KEY_C                 },
		{ KEY_D                 ,     GLFW_KEY_D                 },
		{ KEY_E                 ,     GLFW_KEY_E                 },
		{ KEY_F                 ,     GLFW_KEY_F                 },
		{ KEY_G                 ,     GLFW_KEY_G                 },
		{ KEY_H                 ,     GLFW_KEY_H                 },
		{ KEY_I                 ,     GLFW_KEY_I                 },
		{ KEY_J                 ,     GLFW_KEY_J                 },
		{ KEY_K                 ,     GLFW_KEY_K                 },
		{ KEY_L                 ,     GLFW_KEY_L                 },
		{ KEY_M                 ,     GLFW_KEY_M                 },
		{ KEY_N                 ,     GLFW_KEY_N                 },
		{ KEY_O                 ,     GLFW_KEY_O                 },
		{ KEY_P                 ,     GLFW_KEY_P                 },
		{ KEY_Q                 ,     GLFW_KEY_Q                 },
		{ KEY_R                 ,     GLFW_KEY_R                 },
		{ KEY_S                 ,     GLFW_KEY_S                 },
		{ KEY_T                 ,     GLFW_KEY_T                 },
		{ KEY_U                 ,     GLFW_KEY_U                 },
		{ KEY_V                 ,     GLFW_KEY_V                 },
		{ KEY_W                 ,     GLFW_KEY_W                 },
		{ KEY_X                 ,     GLFW_KEY_X                 },
		{ KEY_Y                 ,     GLFW_KEY_Y                 },
		{ KEY_Z                 ,     GLFW_KEY_Z                 },
		{ KEY_LEFT_BRACKET      ,     GLFW_KEY_LEFT_BRACKET      },
		{ KEY_BACKSLASH         ,     GLFW_KEY_BACKSLASH         },
		{ KEY_RIGHT_BRACKET     ,     GLFW_KEY_RIGHT_BRACKET     },
		{ KEY_GRAVE_ACCENT      ,     GLFW_KEY_GRAVE_ACCENT      },
		{ KEY_WORLD_1           ,     GLFW_KEY_WORLD_1           },
		{ KEY_WORLD_2           ,     GLFW_KEY_WORLD_2           },
		{ KEY_ESCAPE            ,     GLFW_KEY_ESCAPE            },
		{ KEY_ENTER             ,     GLFW_KEY_ENTER             },
		{ KEY_TAB               ,     GLFW_KEY_TAB               },
		{ KEY_BACKSPACE         ,     GLFW_KEY_BACKSPACE         },
		{ KEY_INSERT            ,     GLFW_KEY_INSERT            },
		{ KEY_DELETE            ,     GLFW_KEY_DELETE            },
		{ KEY_RIGHT             ,     GLFW_KEY_RIGHT             },
		{ KEY_LEFT              ,     GLFW_KEY_LEFT              },
		{ KEY_DOWN              ,     GLFW_KEY_DOWN              },
		{ KEY_UP                ,     GLFW_KEY_UP                },
		{ KEY_PAGE_UP           ,     GLFW_KEY_PAGE_UP           },
		{ KEY_PAGE_DOWN         ,     GLFW_KEY_PAGE_DOWN         },
		{ KEY_HOME              ,     GLFW_KEY_HOME              },
		{ KEY_END               ,     GLFW_KEY_END               },
		{ KEY_CAPS_LOCK         ,     GLFW_KEY_CAPS_LOCK         },
		{ KEY_SCROLL_LOCK       ,     GLFW_KEY_SCROLL_LOCK       },
		{ KEY_NUM_LOCK          ,     GLFW_KEY_NUM_LOCK          },
		{ KEY_PRINT_SCREEN      ,     GLFW_KEY_PRINT_SCREEN      },
		{ KEY_PAUSE             ,     GLFW_KEY_PAUSE             },
		{ KEY_F1                ,     GLFW_KEY_F1                },
		{ KEY_F2                ,     GLFW_KEY_F2                },
		{ KEY_F3                ,     GLFW_KEY_F3                },
		{ KEY_F4                ,     GLFW_KEY_F4                },
		{ KEY_F5                ,     GLFW_KEY_F5                },
		{ KEY_F6                ,     GLFW_KEY_F6                },
		{ KEY_F7                ,     GLFW_KEY_F7                },
		{ KEY_F8                ,     GLFW_KEY_F8                },
		{ KEY_F9                ,     GLFW_KEY_F9                },
		{ KEY_F10               ,     GLFW_KEY_F10               },
		{ KEY_F11               ,     GLFW_KEY_F11               },
		{ KEY_F12               ,     GLFW_KEY_F12               },
		{ KEY_F13               ,     GLFW_KEY_F13               },
		{ KEY_F14               ,     GLFW_KEY_F14               },
		{ KEY_F15               ,     GLFW_KEY_F15               },
		{ KEY_F16               ,     GLFW_KEY_F16               },
		{ KEY_F17               ,     GLFW_KEY_F17               },
		{ KEY_F18               ,     GLFW_KEY_F18               },
		{ KEY_F19               ,     GLFW_KEY_F19               },
		{ KEY_F20               ,     GLFW_KEY_F20               },
		{ KEY_F21               ,     GLFW_KEY_F21               },
		{ KEY_F22               ,     GLFW_KEY_F22               },
		{ KEY_F23               ,     GLFW_KEY_F23               },
		{ KEY_F24               ,     GLFW_KEY_F24               },
		{ KEY_F25               ,     GLFW_KEY_F25               },
		{ KEY_KP_0              ,     GLFW_KEY_KP_0              },
		{ KEY_KP_1              ,     GLFW_KEY_KP_1              },
		{ KEY_KP_2              ,     GLFW_KEY_KP_2              },
		{ KEY_KP_3              ,     GLFW_KEY_KP_3              },
		{ KEY_KP_4              ,     GLFW_KEY_KP_4              },
		{ KEY_KP_5              ,     GLFW_KEY_KP_5              },
		{ KEY_KP_6              ,     GLFW_KEY_KP_6              },
		{ KEY_KP_7              ,     GLFW_KEY_KP_7              },
		{ KEY_KP_8              ,     GLFW_KEY_KP_8              },
		{ KEY_KP_9              ,     GLFW_KEY_KP_9              },
		{ KEY_KP_DECIMAL        ,     GLFW_KEY_KP_DECIMAL        },
		{ KEY_KP_DIVIDE         ,     GLFW_KEY_KP_DIVIDE         },
		{ KEY_KP_MULTIPLY       ,     GLFW_KEY_KP_MULTIPLY       },
		{ KEY_KP_SUBTRACT       ,     GLFW_KEY_KP_SUBTRACT       },
		{ KEY_KP_ADD            ,     GLFW_KEY_KP_ADD            },
		{ KEY_KP_ENTER          ,     GLFW_KEY_KP_ENTER          },
		{ KEY_KP_EQUAL          ,     GLFW_KEY_KP_EQUAL          },
		{ KEY_LEFT_SHIFT        ,     GLFW_KEY_LEFT_SHIFT        },
		{ KEY_LEFT_CONTROL      ,     GLFW_KEY_LEFT_CONTROL      },
		{ KEY_LEFT_ALT          ,     GLFW_KEY_LEFT_ALT          },
		{ KEY_LEFT_SUPER        ,     GLFW_KEY_LEFT_SUPER        },
		{ KEY_RIGHT_SHIFT       ,     GLFW_KEY_RIGHT_SHIFT       },
		{ KEY_RIGHT_CONTROL     ,     GLFW_KEY_RIGHT_CONTROL     },
		{ KEY_RIGHT_ALT         ,     GLFW_KEY_RIGHT_ALT         },
		{ KEY_RIGHT_SUPER       ,     GLFW_KEY_RIGHT_SUPER       },
		{ KEY_MENU              ,     GLFW_KEY_MENU              }
	};
	
	inline int32_t KeyCodeToGLFW(KeyCode keycode)
	{
		return KeyCodeToGLFWValue[keycode];
	}

	static std::unordered_map<int32_t, KeyCode> GLFWValueToKeyCode
	{
		{ GLFW_KEY_SPACE             ,     KEY_SPACE             },
		{ GLFW_KEY_APOSTROPHE        ,     KEY_APOSTROPHE        },
		{ GLFW_KEY_COMMA             ,     KEY_COMMA             },
		{ GLFW_KEY_MINUS             ,     KEY_MINUS             },
		{ GLFW_KEY_PERIOD            ,     KEY_PERIOD            },
		{ GLFW_KEY_SLASH             ,     KEY_SLASH             },
		{ GLFW_KEY_0                 ,     KEY_D0                },
		{ GLFW_KEY_1                 ,     KEY_D1                },
		{ GLFW_KEY_2                 ,     KEY_D2                },
		{ GLFW_KEY_3                 ,     KEY_D3                },
		{ GLFW_KEY_4                 ,     KEY_D4                },
		{ GLFW_KEY_5                 ,     KEY_D5                },
		{ GLFW_KEY_6                 ,     KEY_D6                },
		{ GLFW_KEY_7                 ,     KEY_D7                },
		{ GLFW_KEY_8                 ,     KEY_D8                },
		{ GLFW_KEY_9                 ,     KEY_D9                },
		{ GLFW_KEY_SEMICOLON         ,     KEY_SEMICOLON         },
		{ GLFW_KEY_EQUAL             ,     KEY_EQUAL             },
		{ GLFW_KEY_A                 ,     KEY_A                 },
		{ GLFW_KEY_B                 ,     KEY_B                 },
		{ GLFW_KEY_C                 ,     KEY_C                 },
		{ GLFW_KEY_D                 ,     KEY_D                 },
		{ GLFW_KEY_E                 ,     KEY_E                 },
		{ GLFW_KEY_F                 ,     KEY_F                 },
		{ GLFW_KEY_G                 ,     KEY_G                 },
		{ GLFW_KEY_H                 ,     KEY_H                 },
		{ GLFW_KEY_I                 ,     KEY_I                 },
		{ GLFW_KEY_J                 ,     KEY_J                 },
		{ GLFW_KEY_K                 ,     KEY_K                 },
		{ GLFW_KEY_L                 ,     KEY_L                 },
		{ GLFW_KEY_M                 ,     KEY_M                 },
		{ GLFW_KEY_N                 ,     KEY_N                 },
		{ GLFW_KEY_O                 ,     KEY_O                 },
		{ GLFW_KEY_P                 ,     KEY_P                 },
		{ GLFW_KEY_Q                 ,     KEY_Q                 },
		{ GLFW_KEY_R                 ,     KEY_R                 },
		{ GLFW_KEY_S                 ,     KEY_S                 },
		{ GLFW_KEY_T                 ,     KEY_T                 },
		{ GLFW_KEY_U                 ,     KEY_U                 },
		{ GLFW_KEY_V                 ,     KEY_V                 },
		{ GLFW_KEY_W                 ,     KEY_W                 },
		{ GLFW_KEY_X                 ,     KEY_X                 },
		{ GLFW_KEY_Y                 ,     KEY_Y                 },
		{ GLFW_KEY_Z                 ,     KEY_Z                 },
		{ GLFW_KEY_LEFT_BRACKET      ,     KEY_LEFT_BRACKET      },
		{ GLFW_KEY_BACKSLASH         ,     KEY_BACKSLASH         },
		{ GLFW_KEY_RIGHT_BRACKET     ,     KEY_RIGHT_BRACKET     },
		{ GLFW_KEY_GRAVE_ACCENT      ,     KEY_GRAVE_ACCENT      },
		{ GLFW_KEY_WORLD_1           ,     KEY_WORLD_1           },
		{ GLFW_KEY_WORLD_2           ,     KEY_WORLD_2           },
		{ GLFW_KEY_ESCAPE            ,     KEY_ESCAPE            },
		{ GLFW_KEY_ENTER             ,     KEY_ENTER             },
		{ GLFW_KEY_TAB               ,     KEY_TAB               },
		{ GLFW_KEY_BACKSPACE         ,     KEY_BACKSPACE         },
		{ GLFW_KEY_INSERT            ,     KEY_INSERT            },
		{ GLFW_KEY_DELETE            ,     KEY_DELETE            },
		{ GLFW_KEY_RIGHT             ,     KEY_RIGHT             },
		{ GLFW_KEY_LEFT              ,     KEY_LEFT              },
		{ GLFW_KEY_DOWN              ,     KEY_DOWN              },
		{ GLFW_KEY_UP                ,     KEY_UP                },
		{ GLFW_KEY_PAGE_UP           ,     KEY_PAGE_UP           },
		{ GLFW_KEY_PAGE_DOWN         ,     KEY_PAGE_DOWN         },
		{ GLFW_KEY_HOME              ,     KEY_HOME              },
		{ GLFW_KEY_END               ,     KEY_END               },
		{ GLFW_KEY_CAPS_LOCK         ,     KEY_CAPS_LOCK         },
		{ GLFW_KEY_SCROLL_LOCK       ,     KEY_SCROLL_LOCK       },
		{ GLFW_KEY_NUM_LOCK          ,     KEY_NUM_LOCK          },
		{ GLFW_KEY_PRINT_SCREEN      ,     KEY_PRINT_SCREEN      },
		{ GLFW_KEY_PAUSE             ,     KEY_PAUSE             },
		{ GLFW_KEY_F1                ,     KEY_F1                },
		{ GLFW_KEY_F2                ,     KEY_F2                },
		{ GLFW_KEY_F3                ,     KEY_F3                },
		{ GLFW_KEY_F4                ,     KEY_F4                },
		{ GLFW_KEY_F5                ,     KEY_F5                },
		{ GLFW_KEY_F6                ,     KEY_F6                },
		{ GLFW_KEY_F7                ,     KEY_F7                },
		{ GLFW_KEY_F8                ,     KEY_F8                },
		{ GLFW_KEY_F9                ,     KEY_F9                },
		{ GLFW_KEY_F10               ,     KEY_F10               },
		{ GLFW_KEY_F11               ,     KEY_F11               },
		{ GLFW_KEY_F12               ,     KEY_F12               },
		{ GLFW_KEY_F13               ,     KEY_F13               },
		{ GLFW_KEY_F14               ,     KEY_F14               },
		{ GLFW_KEY_F15               ,     KEY_F15               },
		{ GLFW_KEY_F16               ,     KEY_F16               },
		{ GLFW_KEY_F17               ,     KEY_F17               },
		{ GLFW_KEY_F18               ,     KEY_F18               },
		{ GLFW_KEY_F19               ,     KEY_F19               },
		{ GLFW_KEY_F20               ,     KEY_F20               },
		{ GLFW_KEY_F21               ,     KEY_F21               },
		{ GLFW_KEY_F22               ,     KEY_F22               },
		{ GLFW_KEY_F23               ,     KEY_F23               },
		{ GLFW_KEY_F24               ,     KEY_F24               },
		{ GLFW_KEY_F25               ,     KEY_F25               },
		{ GLFW_KEY_KP_0              ,     KEY_KP_0              },
		{ GLFW_KEY_KP_1              ,     KEY_KP_1              },
		{ GLFW_KEY_KP_2              ,     KEY_KP_2              },
		{ GLFW_KEY_KP_3              ,     KEY_KP_3              },
		{ GLFW_KEY_KP_4              ,     KEY_KP_4              },
		{ GLFW_KEY_KP_5              ,     KEY_KP_5              },
		{ GLFW_KEY_KP_6              ,     KEY_KP_6              },
		{ GLFW_KEY_KP_7              ,     KEY_KP_7              },
		{ GLFW_KEY_KP_8              ,     KEY_KP_8              },
		{ GLFW_KEY_KP_9              ,     KEY_KP_9              },
		{ GLFW_KEY_KP_DECIMAL        ,     KEY_KP_DECIMAL        },
		{ GLFW_KEY_KP_DIVIDE         ,     KEY_KP_DIVIDE         },
		{ GLFW_KEY_KP_MULTIPLY       ,     KEY_KP_MULTIPLY       },
		{ GLFW_KEY_KP_SUBTRACT       ,     KEY_KP_SUBTRACT       },
		{ GLFW_KEY_KP_ADD            ,     KEY_KP_ADD            },
		{ GLFW_KEY_KP_ENTER          ,     KEY_KP_ENTER          },
		{ GLFW_KEY_KP_EQUAL          ,     KEY_KP_EQUAL          },
		{ GLFW_KEY_LEFT_SHIFT        ,     KEY_LEFT_SHIFT        },
		{ GLFW_KEY_LEFT_CONTROL      ,     KEY_LEFT_CONTROL      },
		{ GLFW_KEY_LEFT_ALT          ,     KEY_LEFT_ALT          },
		{ GLFW_KEY_LEFT_SUPER        ,     KEY_LEFT_SUPER        },
		{ GLFW_KEY_RIGHT_SHIFT       ,     KEY_RIGHT_SHIFT       },
		{ GLFW_KEY_RIGHT_CONTROL     ,     KEY_RIGHT_CONTROL     },
		{ GLFW_KEY_RIGHT_ALT         ,     KEY_RIGHT_ALT         },
		{ GLFW_KEY_RIGHT_SUPER       ,     KEY_RIGHT_SUPER       },
		{ GLFW_KEY_MENU              ,     KEY_MENU              }
	};

	inline KeyCode GLFWToKeyCode(int32_t glfwKeycode)
	{
		return GLFWValueToKeyCode[glfwKeycode];
	}
}