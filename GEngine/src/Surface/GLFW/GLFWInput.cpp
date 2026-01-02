#include "GEpch.h"
#include "GLFWInput.h"
#include "GEngine/Application.h"
#include <GLFW/glfw3.h>

namespace GEngine
{
	bool GlfwInput::GetKeyPressed(KeyCode keycode)
	{
		return PlatformInput::s_KeyStatesForQuery[keycode].IsPressed;
	}
	bool GlfwInput::GetKeyUp(KeyCode keycode)
	{
		return PlatformInput::s_KeyStatesForQuery[keycode].IsUp;
	}
	bool GlfwInput::GetKeyDown(KeyCode keycode)
	{
		return PlatformInput::s_KeyStatesForQuery[keycode].IsDown == KeyStateInfoForQuery::STATE_UP_TO_DOWN;
	}

	inline bool GlfwInput::GetKeyLongPressed(KeyCode keycode)
	{
		return PlatformInput::s_KeyStatesForQuery[keycode].IsLongPressed;
	}

	inline bool GlfwInput::GetKeyLongUp(KeyCode keycode)
	{
		return PlatformInput::s_KeyStatesForQuery[keycode].IsLongUp;
	}

	inline bool GlfwInput::GetKeyLongDown(KeyCode keycode)
	{
		return PlatformInput::s_KeyStatesForQuery[keycode].IsLongDown == KeyStateInfoForQuery::STATE_UP_TO_DOWN;
	}

	bool GlfwInput::GetMouseButtonPressed(MouseCode button)
	{
		return PlatformInput::s_MouseBtnStatesForQuery[button].IsPressed;
	}
	bool GlfwInput::GetMouseButtonUp(MouseCode button)
	{
		return PlatformInput::s_MouseBtnStatesForQuery[button].IsUp;
	}
	bool GlfwInput::GetMouseButtonDown(MouseCode button)
	{
		return PlatformInput::s_MouseBtnStatesForQuery[button].IsDown == MouseBtnStateInfoForQuery::STATE_UP_TO_DOWN;
	}

	inline bool GlfwInput::GetMouseButtonLongPressed(MouseCode button)
	{
		return PlatformInput::s_MouseBtnStatesForQuery[button].IsLongPressed;
	}

	inline bool GlfwInput::GetMouseButtonLongUp(MouseCode button)
	{
		return PlatformInput::s_MouseBtnStatesForQuery[button].IsLongUp;
	}

	inline bool GlfwInput::GetMouseButtonLongDown(MouseCode button)
	{
		return PlatformInput::s_MouseBtnStatesForQuery[button].IsLongDown == MouseBtnStateInfoForQuery::STATE_UP_TO_DOWN;;
	}

	Vector2 GlfwInput::GetMousePosition()
	{
		auto	window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double	x, y;
		glfwGetCursorPos(window, &x, &y);
		return {(float)x, (float)y};
	}
	float GlfwInput::GetMouseX()
	{
		Vector2		pos = GetMousePosition();
		return pos.x;
	}
	float GlfwInput::GetMouseY()
	{
		Vector2		pos = GetMousePosition();
		return pos.y;
	}
}