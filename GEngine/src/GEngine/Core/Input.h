#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Core/KeyCodes.h"
#include "GEngine/Core/MouseButtonCodes.h"

namespace GEngine
{
	struct KeyStateInfo
	{
		bool IsPressed = false;       // 按键是否处于按下状态
		int PressStartTime = 0;     // 按键按下的时间戳（ms）
		bool IsLongPressTriggered = false; // 长按逻辑是否已触发（避免重复执行）
	};

	struct KeyStateInfoForQuery
	{
		enum DownUpState
		{
			STATE_UP,
			STATE_UP_TO_DOWN,
			STATE_DOWN,
		};
		DownUpState IsDown = STATE_UP;         // 按键在本帧是否被按下
		bool IsUp = false;           // 按键在本帧是否被释放
		bool IsPressed = false;      // 按键是否处于按下状态
		DownUpState IsLongDown = STATE_UP;     // 按键在本帧是否触发长按按下
		bool IsLongUp = false;       // 按键在本帧是否触发长按释放
		bool IsLongPressed = false; // 按键是否处于长按状态
	};

	struct MouseBtnStateInfo
	{
		bool IsPressed = false;       // 按键是否按下
		int PressStartTime = 0;     // 按下时间戳
		float PressX = 0;               // 按下时X坐标
		float PressY = 0;               // 按下时Y坐标
		bool IsLongPressTriggered = false; // 长按是否已触发
	};

	struct MouseBtnStateInfoForQuery
	{
		enum DownUpState
		{
			STATE_UP,
			STATE_UP_TO_DOWN,
			STATE_DOWN,
		};
		DownUpState IsDown = STATE_UP;         // 按键在本帧是否被按下
		bool IsUp = false;           // 按键在本帧是否被释放
		bool IsPressed = false;      // 按键是否处于按下状态
		DownUpState IsLongDown = STATE_UP;     // 按键在本帧是否触发长按按下
		bool IsLongUp = false;       // 按键在本帧是否触发长按释放
		bool IsLongPressed = false; // 按键是否处于长按状态
	};

	class PlatformInput
	{
	public:
		inline virtual bool GetKeyPressed(KeyCode keycode) = 0;
		inline virtual bool GetKeyUp(KeyCode keycode) = 0;
		inline virtual bool GetKeyDown(KeyCode keycode) = 0;
		inline virtual bool GetKeyLongPressed(KeyCode keycode) = 0;
		inline virtual bool GetKeyLongUp(KeyCode keycode) = 0;
		inline virtual bool GetKeyLongDown(KeyCode keycode) = 0;

		inline virtual bool GetMouseButtonPressed(MouseCode button) = 0;
		inline virtual bool GetMouseButtonUp(MouseCode button) = 0;
		inline virtual bool GetMouseButtonDown(MouseCode button) = 0;
		inline virtual bool GetMouseButtonLongPressed(MouseCode button) = 0;
		inline virtual bool GetMouseButtonLongUp(MouseCode button) = 0;
		inline virtual bool GetMouseButtonLongDown(MouseCode button) = 0;

		inline virtual Vector2 GetMousePosition() = 0;
		inline virtual float GetMouseX() = 0;
		inline virtual float GetMouseY() = 0;

		// State storage for event handling
		static std::unordered_map<KeyCode, KeyStateInfo>		s_KeyStates;
		static std::unordered_map<MouseCode, MouseBtnStateInfo> s_MouseBtnStates;

		// State storage for querying in each frame
		static std::unordered_map<KeyCode, KeyStateInfoForQuery>		s_KeyStatesForQuery;
		static std::unordered_map<MouseCode, MouseBtnStateInfoForQuery> s_MouseBtnStatesForQuery;
	};


	class GENGINE_API Input
	{
	public:
		static void Init();

		// true if the key is being held down, false otherwise
		inline static bool GetKeyPressed(KeyCode keycode);
		// true if the key was released this frame
		inline static bool GetKeyUp(KeyCode keycode);
		// true if the key was pressed this frame
		inline static bool GetKeyDown(KeyCode keycode);
		// true if the key has been held down for a long press duration, false otherwise
		inline static bool GetKeyLongPressed(KeyCode keycode);
		// true if the key was released after a long press this frame
		inline static bool GetKeyLongUp(KeyCode keycode);
		// true if the key has just reached the long press duration this frame
		inline static bool GetKeyLongDown(KeyCode keycode);

		// true if the mouse button is being held down, false otherwise
		inline static bool GetMouseButtonPressed(MouseCode button);
		// true if the mouse button was released this frame
		inline static bool GetMouseButtonUp(MouseCode button);
		// true if the mouse button was pressed this frame
		inline static bool GetMouseButtonDown(MouseCode button);
		// true if the mouse button has been held down for a long press duration, false otherwise
		inline static bool GetMouseButtonLongPressed(MouseCode button);
		// true if the mouse button was released after a long press this frame
		inline static bool GetMouseButtonLongUp(MouseCode button);
		// true if the mouse button has just reached the long press duration this frame
		inline static bool GetMouseButtonLongDown(MouseCode button);

		inline static Vector2 GetMousePosition();
		inline static float GetMouseX();
		inline static float GetMouseY();
	private:
		static PlatformInput* s_Instance;
	};

	
	
}