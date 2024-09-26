#pragma once

#include "GEngine/Renderer/GraphicsContext.h"
#include <GLFW/glfw3.h>

namespace GEngine
{
	class GENGINE_API OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);
		virtual ~OpenGLContext() override;

		virtual void Init() override;
		virtual void Uninit() override;
		virtual void SwapBuffers() override;

	private:
		GLFWwindow* m_WindowHandle;
	};
}