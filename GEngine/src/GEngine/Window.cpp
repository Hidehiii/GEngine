#include "GEpch.h"
#include "Window.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Application.h"
#include "Surface/Win32/Win32Window.h"
#include "Surface/GLFW/GLFWWindow.h"
#include "GEngine/Core/Input.h"


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

	void Window::UpdateKeyAndMouseStatesForQuery()
	{
		// record the current key and mouse button states for querying in the next frame
		for (auto& [key, state] : Input::s_KeyStates)
		{
			if (Input::s_KeyStatesForQuery[key].IsDown == KeyStateInfoForQuery::STATE_UP && state.IsPressed)
			{
				Input::s_KeyStatesForQuery[key].IsDown = KeyStateInfoForQuery::STATE_UP_TO_DOWN;
			}
			else if (Input::s_KeyStatesForQuery[key].IsDown == KeyStateInfoForQuery::STATE_UP_TO_DOWN && state.IsPressed)
			{
				Input::s_KeyStatesForQuery[key].IsDown = KeyStateInfoForQuery::STATE_DOWN;
			}
			else if (Input::s_KeyStatesForQuery[key].IsDown == KeyStateInfoForQuery::STATE_DOWN && state.IsPressed == false)
			{
				Input::s_KeyStatesForQuery[key].IsDown = KeyStateInfoForQuery::STATE_UP;
			}
			Input::s_KeyStatesForQuery[key].IsUp = Input::s_KeyStatesForQuery[key].IsPressed == true && state.IsPressed == false;
			if (Input::s_KeyStatesForQuery[key].IsLongDown == KeyStateInfoForQuery::STATE_UP && state.IsPressed && state.IsLongPressTriggered)
			{
				Input::s_KeyStatesForQuery[key].IsLongDown = KeyStateInfoForQuery::STATE_UP_TO_DOWN;
			}
			else if (Input::s_KeyStatesForQuery[key].IsLongDown == KeyStateInfoForQuery::STATE_UP_TO_DOWN && state.IsPressed && state.IsLongPressTriggered)
			{
				Input::s_KeyStatesForQuery[key].IsLongDown = KeyStateInfoForQuery::STATE_DOWN;
			}
			else if (Input::s_KeyStatesForQuery[key].IsLongDown == KeyStateInfoForQuery::STATE_DOWN && (state.IsPressed == false || !state.IsLongPressTriggered))
			{
				Input::s_KeyStatesForQuery[key].IsLongDown = KeyStateInfoForQuery::STATE_UP;
			}
			Input::s_KeyStatesForQuery[key].IsLongUp = Input::s_KeyStatesForQuery[key].IsLongPressed == true && (state.IsPressed == false || !state.IsLongPressTriggered);


			Input::s_KeyStatesForQuery[key].IsPressed = state.IsPressed;
			Input::s_KeyStatesForQuery[key].IsLongPressed = state.IsPressed && state.IsLongPressTriggered;
		}
		for (auto& [button, state] : Input::s_MouseBtnStates)
		{
			if (Input::s_MouseBtnStatesForQuery[button].IsDown == MouseBtnStateInfoForQuery::STATE_UP && state.IsPressed)
			{
				Input::s_MouseBtnStatesForQuery[button].IsDown = MouseBtnStateInfoForQuery::STATE_UP_TO_DOWN;
			}
			else if (Input::s_MouseBtnStatesForQuery[button].IsDown == MouseBtnStateInfoForQuery::STATE_UP_TO_DOWN && state.IsPressed)
			{
				Input::s_MouseBtnStatesForQuery[button].IsDown = MouseBtnStateInfoForQuery::STATE_DOWN;
			}
			else if (Input::s_MouseBtnStatesForQuery[button].IsDown == MouseBtnStateInfoForQuery::STATE_DOWN && state.IsPressed == false)
			{
				Input::s_MouseBtnStatesForQuery[button].IsDown = MouseBtnStateInfoForQuery::STATE_UP;
			}
			Input::s_MouseBtnStatesForQuery[button].IsUp = Input::s_MouseBtnStatesForQuery[button].IsPressed == true && state.IsPressed == false;
			if (Input::s_MouseBtnStatesForQuery[button].IsLongDown == MouseBtnStateInfoForQuery::STATE_UP && state.IsPressed && state.IsLongPressTriggered)
			{
				Input::s_MouseBtnStatesForQuery[button].IsLongDown = MouseBtnStateInfoForQuery::STATE_UP_TO_DOWN;
			}
			else if (Input::s_MouseBtnStatesForQuery[button].IsLongDown == MouseBtnStateInfoForQuery::STATE_UP_TO_DOWN && state.IsPressed && state.IsLongPressTriggered)
			{
				Input::s_MouseBtnStatesForQuery[button].IsLongDown = MouseBtnStateInfoForQuery::STATE_DOWN;
			}
			else if (Input::s_MouseBtnStatesForQuery[button].IsLongDown == MouseBtnStateInfoForQuery::STATE_DOWN && (state.IsPressed == false || !state.IsLongPressTriggered))
			{
				Input::s_MouseBtnStatesForQuery[button].IsLongDown = MouseBtnStateInfoForQuery::STATE_UP;
			}
			Input::s_MouseBtnStatesForQuery[button].IsLongUp = Input::s_MouseBtnStatesForQuery[button].IsLongPressed == true && (state.IsPressed == false || !state.IsLongPressTriggered);

			Input::s_MouseBtnStatesForQuery[button].IsPressed = state.IsPressed;
			Input::s_MouseBtnStatesForQuery[button].IsLongPressed = state.IsPressed && state.IsLongPressTriggered;
		}
	}
}