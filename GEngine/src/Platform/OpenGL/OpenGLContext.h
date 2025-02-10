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
		virtual void SetVSync(bool enable) override { }
		virtual void SetRequiredExtensions(std::vector<const char*> extensions) override { }
	private:
		bool CheckExtensionsSupport();
	private:
		GLFWwindow*					m_WindowHandle;
		std::vector<const char*>	m_Extensions =
		{

		};
	};
}