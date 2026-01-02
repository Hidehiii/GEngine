#include "GEpch.h"
#include "GLFWInput.h"
#include "GEngine/Application.h"
#include <GLFW/glfw3.h>

namespace GEngine
{
	bool GlfwInput::GetKeyPressed(KeyCode keycode)
	{
		auto	window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		int		key = KeyCodeToGLFW(keycode);
		auto	state = glfwGetKey(window, key);
		return state == GLFW_PRESS;
	}
	bool GlfwInput::GetKeyUp(KeyCode keycode)
	{
		auto	window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		int		key = KeyCodeToGLFW(keycode);
		auto	state = glfwGetKey(window, key);
		return state == GLFW_RELEASE && PlatformInput::s_KeyStatesForQuery[keycode].IsPressed;
	}
	bool GlfwInput::GetKeyDown(KeyCode keycode)
	{
		auto	window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		int		key = KeyCodeToGLFW(keycode);
		auto	state = glfwGetKey(window, key);
		return state == GLFW_PRESS && PlatformInput::s_KeyStatesForQuery[keycode].IsPressed == false;
	}

	inline bool GlfwInput::GetKeyLongPressed(KeyCode keycode)
	{
		auto	window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		int		key = KeyCodeToGLFW(keycode);
		auto	state = glfwGetKey(window, key);
		return state == GLFW_PRESS &&
			PlatformInput::s_KeyStatesForQuery[keycode].IsPressed &&
			PlatformInput::s_KeyStatesForQuery[keycode].IsLongPressTriggered;
	}

	inline bool GlfwInput::GetKeyLongUp(KeyCode keycode)
	{
		auto	window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		int		key = KeyCodeToGLFW(keycode);
		auto	state = glfwGetKey(window, key);
		return state == GLFW_RELEASE && 
			PlatformInput::s_KeyStatesForQuery[keycode].IsPressed &&
			PlatformInput::s_KeyStatesForQuery[keycode].IsLongPressTriggered;
	}

	inline bool GlfwInput::GetKeyLongDown(KeyCode keycode)
	{
		auto	window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		int		key = KeyCodeToGLFW(keycode);
		auto	state = glfwGetKey(window, key);
		return state == GLFW_PRESS && 
			PlatformInput::s_KeyStatesForQuery[keycode].IsLongPressTriggered == false && 
			PlatformInput::s_KeyStatesForQuery[keycode].IsPressed &&
			glfwGetTime() - PlatformInput::s_KeyStatesForQuery[keycode].PressStartTime >= Application::Get().GetConfig()->m_LongPressThresholdMs;
	}

	bool GlfwInput::GetMouseButtonPressed(MouseCode button)
	{
		auto	window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		int     btn = MouseCodeToGLFW(button);
		auto	state = glfwGetMouseButton(window, btn);
		return state == GLFW_PRESS;
	}
	bool GlfwInput::GetMouseButtonUp(MouseCode button)
	{
		auto	window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		int     btn = MouseCodeToGLFW(button);
		auto	state = glfwGetMouseButton(window, btn);
		return state == GLFW_RELEASE && PlatformInput::s_MouseBtnStatesForQuery[button].IsPressed;
	}
	bool GlfwInput::GetMouseButtonDown(MouseCode button)
	{
		auto	window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		int     btn = MouseCodeToGLFW(button);
		auto	state = glfwGetMouseButton(window, btn);
		return state == GLFW_PRESS && PlatformInput::s_MouseBtnStatesForQuery[button].IsPressed == false;
	}

	inline bool GlfwInput::GetMouseButtonLongPressed(MouseCode button)
	{
		auto	window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		int     btn = MouseCodeToGLFW(button);
		auto	state = glfwGetMouseButton(window, btn);
		return state == GLFW_PRESS &&
			PlatformInput::s_MouseBtnStatesForQuery[button].IsPressed &&
			PlatformInput::s_MouseBtnStatesForQuery[button].IsLongPressTriggered;
	}

	inline bool GlfwInput::GetMouseButtonLongUp(MouseCode button)
	{
		auto	window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		int     btn = MouseCodeToGLFW(button);
		auto	state = glfwGetMouseButton(window, btn);
		return state == GLFW_RELEASE && 
			PlatformInput::s_MouseBtnStatesForQuery[button].IsPressed &&
			PlatformInput::s_MouseBtnStatesForQuery[button].IsLongPressTriggered;
	}

	inline bool GlfwInput::GetMouseButtonLongDown(MouseCode button)
	{
		auto	window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		int     btn = MouseCodeToGLFW(button);
		auto	state = glfwGetMouseButton(window, btn);
		return state == GLFW_PRESS && 
			PlatformInput::s_MouseBtnStatesForQuery[button].IsLongPressTriggered == false && 
			PlatformInput::s_MouseBtnStatesForQuery[button].IsPressed &&
			glfwGetTime() - PlatformInput::s_MouseBtnStatesForQuery[button].PressStartTime >= Application::Get().GetConfig()->m_LongPressThresholdMs;
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