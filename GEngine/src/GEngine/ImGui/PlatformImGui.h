#pragma once
#include "ImGui/imgui.h"
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/Texture.h"

class GLFWwindow;
namespace GEngine
{
	class GENGINE_API PlatformImGui
	{
	public:
		PlatformImGui() {};
		virtual void OnAttach(GLFWwindow* window) = 0;
		virtual void OnDetach() = 0;

		virtual void Begin() = 0;
		virtual void End() = 0;

		virtual Ref<Texture2D> GetImGuiTexture() = 0;
	};
}