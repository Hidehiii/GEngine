#include "GEpch.h"
#include "Win32Input.h"

namespace GEngine
{
    inline bool Win32Input::GetKeyPressed(KeyCode keycode)
    {
        return false;
    }
    inline bool Win32Input::GetKeyUp(KeyCode keycode)
    {
        return false;
    }
    inline bool Win32Input::GetKeyDown(KeyCode keycode)
    {
        return false;
    }
    inline bool Win32Input::GetKeyLongPressed(KeyCode keycode)
    {
        return false;
    }
    inline bool Win32Input::GetKeyLongUp(KeyCode keycode)
    {
        return false;
    }
    inline bool Win32Input::GetKeyLongDown(KeyCode keycode)
    {
        return false;
    }
    inline bool Win32Input::GetMouseButtonPressed(MouseCode button)
    {
        return false;
    }
    inline bool Win32Input::GetMouseButtonUp(MouseCode button)
    {
        return false;
    }
    inline bool Win32Input::GetMouseButtonDown(MouseCode button)
    {
        return false;
    }
    inline bool Win32Input::GetMouseButtonLongPressed(MouseCode button)
    {
        return false;
    }
    inline bool Win32Input::GetMouseButtonLongUp(MouseCode button)
    {
        return false;
    }
    inline bool Win32Input::GetMouseButtonLongDown(MouseCode button)
    {
        return false;
    }
    inline Vector2 Win32Input::GetMousePosition()
    {
        return Vector2();
    }
    inline float Win32Input::GetMouseX()
    {
        return 0.0f;
    }
    inline float Win32Input::GetMouseY()
    {
        return 0.0f;
    }
}