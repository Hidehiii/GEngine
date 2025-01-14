#pragma once
#include <GEngine.h>
#include <imgui_internal.h>

namespace GEngine
{
	class GUI
	{
	public:
		static void FloatControl(const std::string& label, float& val, float restVal = 0.0f, float speed = 0.1f, float size = 100.0f);
		static void Vector2Control(const std::string& label, Vector2& val, Vector2 restVal = Vector2(0.0f), float speed = 0.1f, float size = 100.0f);
		static void Vector3Control(const std::string& label, Vector3& val, Vector3 restVal = Vector3(0.0f), float speed = 0.1f, float size = 100.0f);
		static void Vector4Control(const std::string& label, Vector4& val, Vector4 restVal = Vector4(0.0f), float speed = 0.1f, float size = 100.0f);
		static void Color4Control(const std::string& label, Vector4& color, float size = 100.0f);
		static void Checkbox(const std::string& label, bool& val, float size = 100.0f);
		static void IntControl(const std::string& label, int& val, int restVal = 0, int speed = 1, float size = 100.0f);
	};
}