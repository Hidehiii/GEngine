#pragma once

#include "GEngine/ImGui/PlatformImGui.h"

class GLFWwindow;
namespace GEngine
{
	class GENGINE_API OpenGLImGui : public PlatformImGui
	{
	public:
		OpenGLImGui() {}
		virtual void OnAttach(GLFWwindow* window) override;
		virtual void OnDetach() override;

		virtual void Begin() override;
		virtual void End() override;
	};
}


