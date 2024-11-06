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

		virtual void Init(const unsigned int width, const unsigned int height) override;
		virtual void Uninit() override;
		virtual void SwapBuffers() override;
		virtual void SetRequiredExtensions(std::vector<const char*> extensions) override { }
		virtual void RecreateSwapChain(unsigned int width, unsigned int height) override { }
	private:
		GLFWwindow* m_WindowHandle;
	};
}