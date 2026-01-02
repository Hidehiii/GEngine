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
	}
	void InputSystemExplame::OnImGuiRender()
	{
	}
	void InputSystemExplame::OnEvent(Event& e)
	{
	}
}