#include "GEpch.h"
#include "GEngine/Core/Input.h"
#include "GEngine/Application.h"
#include <GLFW/glfw3.h>

namespace GEngine
{
#ifdef GE_PLATFORM_WINDOWS
	bool Input::IsKeyPressed(int keycode)
	{
		auto	window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto	state = glfwGetKey(window, keycode);
		return state == GLFW_PRESS;
	}
	bool Input::IsKeyReleased(int keycode)
	{
		auto	window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto	state = glfwGetKey(window, keycode);
		return state == GLFW_RELEASE;
	}

	bool Input::IsMouseButtonPressed(int button)
	{
		auto	window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto	state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}
	bool Input::IsMouseButtonReleased(int button)
	{
		auto	window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto	state = glfwGetMouseButton(window, button);
		return state == GLFW_RELEASE;
	}

	Vector2 Input::GetMousePosition()
	{
		auto	window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double	x, y;
		glfwGetCursorPos(window, &x, &y);
		return {(float)x, (float)y};
	}
	float Input::GetMouseX()
	{
		Vector2		pos = GetMousePosition();
		return pos.x;
	}
	float Input::GetMouseY()
	{
		Vector2		pos = GetMousePosition();
		return pos.y;
	}
#endif
}