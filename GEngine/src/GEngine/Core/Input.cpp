#include "GEpch.h"
#include "Input.h"
#include "GEngine/Graphics/Graphics.h"
#include "Surface/GLFW/GLFWInput.h"

namespace GEngine
{
	PlatformInput* Input::s_Instance = nullptr;
	std::unordered_map<KeyCode, KeyStateInfo> PlatformInput::s_KeyStates;
	std::unordered_map<MouseCode, MouseBtnStateInfo> PlatformInput::s_MouseBtnStates;
	std::unordered_map<KeyCode, KeyStateInfo> PlatformInput::s_KeyStatesForQuery;
	std::unordered_map<MouseCode, MouseBtnStateInfo> PlatformInput::s_MouseBtnStatesForQuery;
	void Input::Init()
	{
		switch (GraphicsAPI::GetAPI())
		{
		case GRAPHICS_API_NONE:
			GE_CORE_ASSERT(false, "GRAPHICS_API_NONE is not supported!");
			return;
		case GRAPHICS_API_OPENGL:
		case GRAPHICS_API_VULKAN:
			s_Instance = new GlfwInput();
			break;
		default:
			GE_CORE_ASSERT(false, "Unknown Graphics API!");
			return;
		}
	}

	bool Input::GetKeyPressed(KeyCode keycode)
	{
		return s_Instance->GetKeyPressed(keycode);
	}
	bool Input::GetKeyUp(KeyCode keycode)
	{
		return s_Instance->GetKeyUp(keycode);
	}
	bool Input::GetKeyDown(KeyCode keycode)
	{
		return s_Instance->GetKeyDown(keycode);
	}
	bool Input::GetKeyLongPressed(KeyCode keycode)
	{
		return s_Instance->GetKeyLongPressed(keycode);
	}
	bool Input::GetKeyLongUp(KeyCode keycode)
	{
		return s_Instance->GetKeyLongUp(keycode);
	}
	bool Input::GetKeyLongDown(KeyCode keycode)
	{
		return s_Instance->GetKeyLongDown(keycode);
	}
	bool Input::GetMouseButtonPressed(MouseCode button)
	{
		return s_Instance->GetMouseButtonPressed(button);
	}
	bool Input::GetMouseButtonUp(MouseCode button)
	{
		return s_Instance->GetMouseButtonUp(button);
	}
	bool Input::GetMouseButtonDown(MouseCode button)
	{
		return s_Instance->GetMouseButtonDown(button);
	}
	bool Input::GetMouseButtonLongPressed(MouseCode button)
	{
		return s_Instance->GetMouseButtonLongPressed(button);
	}
	bool Input::GetMouseButtonLongUp(MouseCode button)
	{
		return s_Instance->GetMouseButtonLongUp(button);
	}
	bool Input::GetMouseButtonLongDown(MouseCode button)
	{
		return s_Instance->GetMouseButtonLongDown(button);
	}
	Vector2 Input::GetMousePosition()
	{
		return s_Instance->GetMousePosition();
	}
	float Input::GetMouseX()
	{
		return s_Instance->GetMouseX();
	}
	float Input::GetMouseY()
	{
		return s_Instance->GetMouseY();
	}
}