#include "GEpch.h"
#include "OpenGLContext.h"
#include "OpenGLCommandBuffer.h"
#include "GEngine/Renderer/Renderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace GEngine
{
	OpenGLContext* OpenGLContext::s_ContextInstance = nullptr;

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		GE_CORE_ASSERT(windowHandle, "Window handle is null!");
		s_ContextInstance = this;
	}
	OpenGLContext::~OpenGLContext()
	{
	}
	void OpenGLContext::Init(const unsigned int width, const unsigned int height)
	{
		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		GE_CORE_ASSERT(status, "Failed to initialize Glad!");

		GE_CORE_INFO("OpenGL Info:");
		GE_CORE_INFO("    OpenGL Vender {0}:", (char*)glGetString(GL_VENDOR));
		GE_CORE_INFO("    OpenGL Renderer {0}:", (char*)glGetString(GL_RENDERER));
		GE_CORE_INFO("    OpenGL Version {0}:", (char*)glGetString(GL_VERSION));

		CheckExtensionsSupport();
		CreateCommandBuffers();
	}
	void OpenGLContext::Uninit()
	{
	}
	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}
	Ref<OpenGLCommandBuffer> OpenGLContext::GetCommandBuffer(CommandBufferType type)
	{
		if (type == CommandBufferType::Graphics)
			return m_GraphicsCommandBuffers.at(m_GraphicsCommandBufferIndex++ % m_GraphicsCommandBuffers.size());
		if (type == CommandBufferType::Compute)
			return m_ComputeCommandBuffers.at(m_ComputeCommandBufferIndex++ % m_ComputeCommandBuffers.size());
		return nullptr;
	}
	bool OpenGLContext::CheckExtensionsSupport()
	{
		GLint numExtensions;
		glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
		for (int i = 0; i < m_Extensions.size(); i++)
		{
			bool support = false;
			for (GLint i = 0; i < numExtensions; i++) {
				const char* currentExt = (const char*)glGetStringi(GL_EXTENSIONS, i);
				if (strcmp(currentExt, m_Extensions.at(i)) == 0) {
					support = true;
					break;
				}
			}
			if (support == false)
			{
				return false;
			}
		}
		return true;
	}
	void OpenGLContext::CreateCommandBuffers()
	{
		for (int i = 0; i < Renderer::GetCommandBufferCount(); i++)
		{
			m_GraphicsCommandBuffers.push_back(OpenGLCommandBuffer::Create(CommandBufferType::Graphics));
			m_ComputeCommandBuffers.push_back(OpenGLCommandBuffer::Create(CommandBufferType::Compute));
		}
	}

}