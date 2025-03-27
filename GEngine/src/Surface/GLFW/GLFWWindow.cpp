#include "GEpch.h"
#include "GLFWWindow.h"
#include "GEngine/Graphics/GraphicsAPI.h"
#include "Platform/OpenGL/OpenGLContext.h"
#include "Platform/Vulkan/VulkanContext.h"


namespace GEngine
{
	static bool s_GLFWInitialized = false;

	static void GLFWErrorCallback(int error, const char* description)
	{
		GE_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	GLFWWindow::GLFWWindow(const WindowProps& props)
	{
		Init(props);
	}

	GLFWWindow::~GLFWWindow()
	{
		
		Shutdown();
	}

	void GLFWWindow::Init(const WindowProps& props)
	{
		

		m_Data.Title	= props.Title;
		m_Data.Width	= props.Width;
		m_Data.Height	= props.Height;

		GE_CORE_TRACE("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

		if (!s_GLFWInitialized)
		{
			int success			= glfwInit();
			GE_CORE_ASSERT(success, "Could not initialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);
			s_GLFWInitialized	= true;
		}	
		if (GraphicsAPI::GetAPI() == GraphicsAPI::API::OpenGL)
		{
			
		}
		else if (GraphicsAPI::GetAPI() == GraphicsAPI::API::Vulkan)
		{
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}
		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);

		if (GraphicsAPI::GetAPI() == GraphicsAPI::API::OpenGL)
		{
			m_Context = new OpenGLContext(m_Window);
		}
		else if (GraphicsAPI::GetAPI() == GraphicsAPI::API::Vulkan)
		{
			m_Context = new VulkanContext(m_Window);
			m_Context->SetRequiredExtensions(GetRequiredExtensions());
		}
		else
		{
			GE_CORE_ASSERT(false, "Renderer API not supported");
		}
			

		
		m_Context->Init(m_Data.Width, m_Data.Height);

		
		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		//set GLFW callbacks
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data	= *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width			= width;
			data.Height			= height;

			WindowResizeEvent	event(width, height);
			data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
			case GLFW_PRESS:
			{
				KeyPressedEvent event(key, 0);
				data.EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent event(key);
				data.EventCallback(event);
				break;
			}
			case GLFW_REPEAT:
			{
				KeyPressedEvent event(key, true);
				data.EventCallback(event);
				break;
			}
			}
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
			{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			KeyTypedEvent event(keycode);
			data.EventCallback(event);
		});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
			{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
			case GLFW_PRESS:
			{
				MouseButtonPressedEvent event(button);
				data.EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				MouseButtonReleasedEvent event(button);
				data.EventCallback(event);
				break;
			}
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
			{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
			{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
		});
	}

	void GLFWWindow::Shutdown()
	{
		glfwDestroyWindow(m_Window);
		m_Context->Uninit();
	}

	void GLFWWindow::OnUpdate()
	{
		glfwPollEvents();
		m_Context->SwapBuffers();
	}

	// only used for Vulkan
	std::vector<const char*> GLFWWindow::GetRequiredExtensions()
	{
		uint32_t					glfwExtensionCount = 0;
		const char**				glfwExtensions;
		glfwExtensions				= glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*>	extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		return extensions;
	}

	void GLFWWindow::SetVSync(bool enabled)
	{
		if (GraphicsAPI::GetAPI() == GraphicsAPI::API::OpenGL)
		{
			if (enabled)
				glfwSwapInterval(1);
			else
				glfwSwapInterval(0);
		}

		m_Context->SetVSync(enabled);
		m_Data.VSync = enabled;
	}

	bool GLFWWindow::IsVSync() const
	{
		return m_Data.VSync;
	}
	float GLFWWindow::GetTime() const
	{
		return (float)glfwGetTime();
	}
}