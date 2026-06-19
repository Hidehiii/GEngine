#include "GEpch.h"
#include "Input.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Application.h"
#include "Surface/GLFW/GLFWInput.h"
#include "Surface/Win32/Win32Input.h"

namespace GEngine
{
	std::pair<MouseCode, float>									Input::s_LastMouseBtnClicked = { MouseCode::MOUSE_BUTTON_UNKNOWN, 0.0f };
	float														Input::s_MouseX = 0.0f;
	float														Input::s_MouseY = 0.0f;
	std::unordered_map<KeyCode, KeyStateInfo>					Input::s_KeyStates;
	std::unordered_map<MouseCode, MouseBtnStateInfo>			Input::s_MouseBtnStates;
	std::unordered_map<KeyCode, KeyStateInfoForQuery>			Input::s_KeyStatesForQuery;
	std::unordered_map<MouseCode, MouseBtnStateInfoForQuery>	Input::s_MouseBtnStatesForQuery;
	void Input::Init()
	{
		GE_CORE_INFO("Initializing Input");
	}

	bool Input::GetKeyPressed(KeyCode keycode)
	{
		return s_KeyStatesForQuery[keycode].IsPressed;
	}
	bool Input::GetKeyUp(KeyCode keycode)
	{
		return s_KeyStatesForQuery[keycode].IsUp;
	}
	bool Input::GetKeyDown(KeyCode keycode)
	{
		return s_KeyStatesForQuery[keycode].IsDown == KeyStateInfoForQuery::STATE_UP_TO_DOWN;
	}
	bool Input::GetKeyLongPressed(KeyCode keycode)
	{
		return s_KeyStatesForQuery[keycode].IsLongPressed;
	}
	bool Input::GetKeyLongUp(KeyCode keycode)
	{
		return s_KeyStatesForQuery[keycode].IsLongUp;
	}
	bool Input::GetKeyLongDown(KeyCode keycode)
	{
		return s_KeyStatesForQuery[keycode].IsLongDown == KeyStateInfoForQuery::STATE_UP_TO_DOWN;
	}
	bool Input::GetMouseButtonPressed(MouseCode button)
	{
		return s_MouseBtnStatesForQuery[button].IsPressed;
	}
	bool Input::GetMouseButtonUp(MouseCode button)
	{
		return s_MouseBtnStatesForQuery[button].IsUp;
	}
	bool Input::GetMouseButtonDown(MouseCode button)
	{
		return s_MouseBtnStatesForQuery[button].IsDown == MouseBtnStateInfoForQuery::STATE_UP_TO_DOWN;
	}
	bool Input::GetMouseButtonLongPressed(MouseCode button)
	{
		return s_MouseBtnStatesForQuery[button].IsLongPressed;
	}
	bool Input::GetMouseButtonLongUp(MouseCode button)
	{
		return s_MouseBtnStatesForQuery[button].IsLongUp;
	}
	bool Input::GetMouseButtonLongDown(MouseCode button)
	{
		return s_MouseBtnStatesForQuery[button].IsLongDown == MouseBtnStateInfoForQuery::STATE_UP_TO_DOWN;
	}
	Vector2 Input::GetMousePosition()
	{
		return { s_MouseX, s_MouseY };
	}
	float Input::GetMouseX()
	{
		return s_MouseX;
	}
	float Input::GetMouseY()
	{
		return s_MouseY;
	}
}