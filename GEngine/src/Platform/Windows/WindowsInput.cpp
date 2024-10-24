#include "GEpch.h"
#include "GEngine/Core/Input.h"
#include "GEngine/Application.h"
#include <GLFW/glfw3.h>

namespace GEngine
{

	bool Input::IsKeyPressed(int keycode)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, keycode);
		return state == GLFW_PRESS;
	}
	bool Input::IsMouseButtonPressed(int button)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}
	Vector2 Input::GetMousePosition()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		return {(float)x, (float)y};
	}
	float Input::GetMouseX()
	{
		Vector2 pos = GetMousePosition();
		return pos.x;
	}
	float Input::GetMouseY()
	{
		Vector2 pos = GetMousePosition();
		return pos.y;
	}
}