#include "InputSystemExplame.h"

namespace GEngine
{
	InputSystemExplame::InputSystemExplame()
	{
	}
	void InputSystemExplame::OnAttach()
	{
	}
	void InputSystemExplame::OnPresent()
	{
	}
	void InputSystemExplame::OnRender()
	{
	}
	void InputSystemExplame::OnUpdate()
	{
		if(Input::GetKeyPressed(KEY_A))
		{
			GE_INFO("Key A is pressed!");
		}
		if (Input::GetKeyDown(KEY_B))
		{
			GE_INFO("Key B is down!");
		}
		if (Input::GetKeyUp(KEY_C))
		{
			GE_INFO("Key C is up!");
		}
		if (Input::GetMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			GE_INFO("Left Mouse Button is pressed at position: ({0}, {1})", Input::GetMouseX(), Input::GetMouseY());
		}
		if(Input::GetMouseButtonDown(MOUSE_BUTTON_RIGHT))
		{
			GE_INFO("Right Mouse Button is down at position: ({0}, {1})", Input::GetMouseX(), Input::GetMouseY());
		}
		if(Input::GetMouseButtonUp(MOUSE_BUTTON_MIDDLE))
		{
			GE_INFO("Middle Mouse Button is up at position: ({0}, {1})", Input::GetMouseX(), Input::GetMouseY());
		}
		if(Input::GetKeyLongPressed(KEY_D))
		{
			GE_INFO("Key D is long pressed!");
		}
		if(Input::GetMouseButtonLongPressed(MOUSE_BUTTON_0))
		{
			GE_INFO("Mouse Button 0 is long pressed at position: ({0}, {1})", Input::GetMouseX(), Input::GetMouseY());
		}
		if(Input::GetKeyLongDown(KEY_E))
		{
			GE_INFO("Key E has just reached long press duration!");
		}
		if(Input::GetMouseButtonLongDown(MOUSE_BUTTON_1))
		{
			GE_INFO("Mouse Button 1 has just reached long press duration at position: ({0}, {1})", Input::GetMouseX(), Input::GetMouseY());
		}
		if(Input::GetKeyLongUp(KEY_F))
		{
			GE_INFO("Key F was released after a long press!");
		}
		if(Input::GetMouseButtonLongUp(MOUSE_BUTTON_2))
		{
			GE_INFO("Mouse Button 2 was released after a long press at position: ({0}, {1})", Input::GetMouseX(), Input::GetMouseY());
		}
	}
	void InputSystemExplame::OnImGuiRender()
	{
	}
	void InputSystemExplame::OnEvent(Event& e)
	{
		EventDispatcher::Dispatch<KeyPressedEvent>(e, [&](KeyPressedEvent& event)
		{
			GE_INFO("Key Pressed Event: {}", event.GetKeyCode());
			return false;
			});
		EventDispatcher::Dispatch<KeyUpEvent>(e, [&](KeyUpEvent& event)
		{
			GE_INFO("Key Released Event: {}", event.GetKeyCode());
			return false;
			});
		EventDispatcher::Dispatch<MouseButtonPressedEvent>(e, [&](MouseButtonPressedEvent& event)
		{
			GE_INFO("Mouse Button Pressed Event: {}", event.GetMouseButton());
			return false;
			});
		EventDispatcher::Dispatch<MouseButtonUpEvent>(e, [&](MouseButtonUpEvent& event)
		{
			GE_INFO("Mouse Button Released Event: {}", event.GetMouseButton());
			return false;
			});
	}
}