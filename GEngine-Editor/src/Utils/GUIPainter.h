#pragma once
#include <GEngine.h>
#include <imgui_internal.h>

namespace GEngine
{
	class GUIPainter
	{
	public:
		static void DrawFloatControl(const std::string& label, float& val, float restVal = 0.0f, float speed = 0.1f, float size = 100.0f);
		static void DrawVector2Control(const std::string& label, Vector2& val, Vector2 restVal = Vector2(0.0f), float speed = 0.1f, float size = 100.0f);
		static void DrawVector3Control(const std::string& label, Vector3& val, Vector3 restVal = Vector3(0.0f), float speed = 0.1f, float size = 100.0f);
		static void DrawVector4Control(const std::string& label, Vector4& val, Vector4 restVal = Vector4(0.0f), float speed = 0.1f, float size = 100.0f);
		static void DrawColor4Control(const std::string& label, Vector4& color, float size = 100.0f); 
		static void DrawCheckbox(const std::string& label, bool& val, float size = 100.0f);
	};
}