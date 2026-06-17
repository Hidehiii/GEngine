#include "GEpch.h"
#include "Window.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Application.h"
#include "Surface/Win32/Win32Window.h"
#include "Surface/GLFW/GLFWWindow.h"


namespace GEngine
{
	Window* Window::Create(const WindowProps& props)
	{
		switch (Application::Get().GetConfig()->GetWindowManagerAPI())
		{
		case Config::CONFIG_WINDOW_MANAGER_API_GLFW:	return new GLFWWindow(props);
		case Config::CONFIG_WINDOW_MANAGER_API_WIN32:	return new Win32Window(props);
		default:
			break;
		}
		return nullptr;
	}
}