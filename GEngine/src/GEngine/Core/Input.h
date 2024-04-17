#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Core/KeyCodes.h"
#include "GEngine/Core/MouseButtonCodes.h"

namespace GEngine
{
	class GENGINE_API Input
	{
	public:
		inline static bool IsKeyPressed(int keycode);

		inline static bool IsMouseButtonPressed(int button);
		inline static Vector2 GetMousePosition();
		inline static float GetMouseX();
		inline static float GetMouseY();
	};
}