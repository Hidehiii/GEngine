#include "GEpch.h"
#include "GLFWInput.h"
#include "GEngine/Application.h"
#include <GLFW/glfw3.h>

namespace GEngine
{
	bool GLFWInput::GetKeyPressed(KeyCode keycode)
	{
		return PlatformInput::s_KeyStatesForQuery[keycode].IsPressed;
	}
	bool GLFWInput::GetKeyUp(KeyCode keycode)
	{
		return PlatformInput::s_KeyStatesForQuery[keycode].IsUp;
	}
	bool GLFWInput::GetKeyDown(KeyCode keycode)
	{
		return PlatformInput::s_KeyStatesForQuery[keycode].IsDown == KeyStateInfoForQuery::STATE_UP_TO_DOWN;
	}

	inline bool GLFWInput::GetKeyLongPressed(KeyCode keycode)
	{
		return PlatformInput::s_KeyStatesForQuery[keycode].IsLongPressed;
	}

	inline bool GLFWInput::GetKeyLongUp(KeyCode keycode)
	{
		return PlatformInput::s_KeyStatesForQuery[keycode].IsLongUp;
	}

	inline bool GLFWInput::GetKeyLongDown(KeyCode keycode)
	{
		return PlatformInput::s_KeyStatesForQuery[keycode].IsLongDown == KeyStateInfoForQuery::STATE_UP_TO_DOWN;
	}

	bool GLFWInput::GetMouseButtonPressed(MouseCode button)
	{
		return PlatformInput::s_MouseBtnStatesForQuery[button].IsPressed;
	}
	bool GLFWInput::GetMouseButtonUp(MouseCode button)
	{
		return PlatformInput::s_MouseBtnStatesForQuery[button].IsUp;
	}
	bool GLFWInput::GetMouseButtonDown(MouseCode button)
	{
		return PlatformInput::s_MouseBtnStatesForQuery[button].IsDown == MouseBtnStateInfoForQuery::STATE_UP_TO_DOWN;
	}

	inline bool GLFWInput::GetMouseButtonLongPressed(MouseCode button)
	{
		return PlatformInput::s_MouseBtnStatesForQuery[button].IsLongPressed;
	}

	inline bool GLFWInput::GetMouseButtonLongUp(MouseCode button)
	{
		return PlatformInput::s_MouseBtnStatesForQuery[button].IsLongUp;
	}

	inline bool GLFWInput::GetMouseButtonLongDown(MouseCode button)
	{
		return PlatformInput::s_MouseBtnStatesForQuery[button].IsLongDown == MouseBtnStateInfoForQuery::STATE_UP_TO_DOWN;;
	}

	Vector2 GLFWInput::GetMousePosition()
	{
		auto	window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double	x, y;
		glfwGetCursorPos(window, &x, &y);
		return {(float)x, (float)y};
	}
	float GLFWInput::GetMouseX()
	{
		Vector2		pos = GetMousePosition();
		return pos.x;
	}
	float GLFWInput::GetMouseY()
	{
		Vector2		pos = GetMousePosition();
		return pos.y;
	}
}