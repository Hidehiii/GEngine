#pragma once
#include "GEngine/Core/Input.h"

namespace GEngine
{
	class GENGINE_API GlfwInput : public PlatformInput
	{
	public:
		inline virtual bool GetKeyPressed(KeyCode keycode) override;
		inline virtual bool GetKeyUp(KeyCode keycode) override;
		inline virtual bool GetKeyDown(KeyCode keycode) override;
		inline virtual bool GetKeyLongPressed(KeyCode keycode) override;
		inline virtual bool GetKeyLongUp(KeyCode keycode) override;
		inline virtual bool GetKeyLongDown(KeyCode keycode) override;
			
		inline virtual bool GetMouseButtonPressed(MouseCode button) override;
		inline virtual bool GetMouseButtonUp(MouseCode button) override;
		inline virtual bool GetMouseButtonDown(MouseCode button) override;
		inline virtual bool GetMouseButtonLongPressed(MouseCode button) override;
		inline virtual bool GetMouseButtonLongUp(MouseCode button) override;
		inline virtual bool GetMouseButtonLongDown(MouseCode button) override;
			
		inline virtual Vector2 GetMousePosition() override;
		inline virtual float GetMouseX() override;
		inline virtual float GetMouseY() override;
	};
}