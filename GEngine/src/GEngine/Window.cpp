#include "GEpch.h"
#include "Window.h"
#include "GEngine/Graphics/Graphics.h"
#include "Surface/Win32/Win32Window.h"
#include "Surface/GLFW/GLFWWindow.h"


namespace GEngine
{
	Window* Window::Create(const WindowProps& props)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GraphicsAPI::API::OpenGL:		return new GLFWWindow(props);
		case GraphicsAPI::API::Vulkan:		return new GLFWWindow(props);
		case GraphicsAPI::API::Direct3DX12: return new Win32Window(props);
		default:
			break;
		}
		return nullptr;
	}
}