#include "GEpch.h"
#include "GLFWWindow.h"
#include "GEngine/Graphics/GraphicsAPI.h"
#include "Platform/OpenGL/OpenGLContext.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "GEngine/Core/Input.h"
#include "GEngine/Application.h"


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
		if (GraphicsAPI::GetAPI() == GRAPHICS_API_OPENGL)
		{
			
		}
		else if (GraphicsAPI::GetAPI() == GRAPHICS_API_VULKAN)
		{
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}
		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);

		if (GraphicsAPI::GetAPI() == GRAPHICS_API_OPENGL)
		{
			m_Context = new OpenGLContext(m_Window);
		}
		else if (GraphicsAPI::GetAPI() == GRAPHICS_API_VULKAN)
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
				KeyCode keycode = GLFWToKeyCode(key);

				KeyDownEvent downEvent(keycode);
				data.EventCallback(downEvent);

				KeyPressedEvent event(keycode);
				data.EventCallback(event);
				
				KeyStateInfo& keyState = PlatformInput::s_KeyStates[keycode];

				keyState.IsPressed = true;
				keyState.PressStartTime = int(glfwGetTime() * 1000);
				keyState.IsLongPressTriggered = false;
				break;
			}
			case GLFW_RELEASE:
			{
				KeyCode keycode = GLFWToKeyCode(key);
				KeyStateInfo& keyState = PlatformInput::s_KeyStates[keycode];

				KeyUpEvent event(keycode);
				data.EventCallback(event);

				if(keyState.IsPressed)
				{
					if(keyState.IsLongPressTriggered)
					{
						// 长按已触发，执行长按释放逻辑（如果有的话）
						KeyLongUpEvent longUpEvent(keycode);
						data.EventCallback(longUpEvent);
					}
					else
					{
						// 普通按键释放逻辑
						KeyClickEvent clickEvent(keycode);
						data.EventCallback(clickEvent);
					}

					keyState.IsPressed = false;
					keyState.IsLongPressTriggered = false;
				}

				
				break;
			}
			case GLFW_REPEAT:
			{
				KeyCode keycode = GLFWToKeyCode(key);
				KeyStateInfo& keyState = PlatformInput::s_KeyStates[keycode];

				if(keyState.IsPressed == false)
				{
					// 如果按键状态为未按下，忽略重复事件
					break;
				}
				KeyPressedEvent event(keycode);
				data.EventCallback(event);

				
				if(!keyState.IsLongPressTriggered)
				{
					if (Application::Get().GetConfig()->m_LongPressThresholdMs <= int(glfwGetTime() * 1000) - keyState.PressStartTime)
					{
						KeyLongDownEvent longDownEvent(keycode);
						data.EventCallback(longDownEvent);

						// 触发长按事件
						KeyLongPressedEvent longPressEvent(keycode);
						data.EventCallback(longPressEvent);

						keyState.IsLongPressTriggered = true;
					}
				}
				else
				{
					KeyLongPressedEvent longPressEvent(keycode);
					data.EventCallback(longPressEvent);
				}
				break;
			}
			}
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
			{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			KeyCode key = GLFWToKeyCode(keycode);

			KeyTypedEvent event(key);
			data.EventCallback(event);
		});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
			{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
			case GLFW_PRESS:
			{
				MouseCode buttonCode = (MouseCode)button;
				MouseBtnStateInfo& btnState = PlatformInput::s_MouseBtnStates[buttonCode];

				MouseButtonDownEvent downEvent(buttonCode);
				data.EventCallback(downEvent);

				MouseButtonPressedEvent event(buttonCode);
				data.EventCallback(event);

				btnState.IsPressed = true;
				btnState.PressStartTime = int(glfwGetTime() * 1000);
				double x, y = 0;
				glfwGetCursorPos(window, &x, &y);
				btnState.PressX = (float)x;
				btnState.PressY = (float)y;
				btnState.IsLongPressTriggered = false;
				break;
			}
			case GLFW_RELEASE:
			{
				MouseCode buttonCode = (MouseCode)button;
				MouseBtnStateInfo& btnState = PlatformInput::s_MouseBtnStates[buttonCode];

				MouseButtonUpEvent event(buttonCode);
				data.EventCallback(event);

				if(btnState.IsPressed)
				{
					if(btnState.IsLongPressTriggered)
					{
						// 长按已触发，执行长按释放逻辑（如果有的话）
						MouseButtonLongUpEvent longUpEvent(buttonCode);
						data.EventCallback(longUpEvent);
					}
					else
					{
						// 普通按键释放逻辑
						MouseButtonClickEvent clickEvent(buttonCode);
						data.EventCallback(clickEvent);
					}
					btnState.IsPressed = false;
					btnState.IsLongPressTriggered = false;
				}
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

	void GLFWWindow::MouseButtonPreessedCallback()
	{
		for(auto& [button, btnState] : PlatformInput::s_MouseBtnStates)
		{
			int btnCode = MouseCodeToGLFW(button);
			auto state = glfwGetMouseButton(m_Window, btnCode);
			if(state == GLFW_RELEASE)
				continue;
			if(state == GLFW_PRESS)
			{
				if(btnState.IsPressed == false)
				{
					continue;
				}
				else
				{
					// 按钮仍然按下，继续处理长按逻辑
					MouseButtonPressedEvent event(button);
					WindowData& data = *(WindowData*)glfwGetWindowUserPointer(m_Window);
					data.EventCallback(event);

					if (!btnState.IsLongPressTriggered)
					{
						if (Application::Get().GetConfig()->m_LongPressThresholdMs <= int(glfwGetTime() * 1000) - btnState.PressStartTime)
						{
							MouseButtonLongDownEvent longDownEvent(button);
							data.EventCallback(longDownEvent);
							// 触发长按事件
							MouseButtonLongPressedEvent longPressEvent(button);
							data.EventCallback(longPressEvent);
							btnState.IsLongPressTriggered = true;
						}
					}
					else
					{
						MouseButtonLongPressedEvent longPressEvent(button);
						data.EventCallback(longPressEvent);
					}
				}
			}
		}
	}

	void GLFWWindow::UpdateKeyAndMouseStatesForQuery()
	{
		// record the current key and mouse button states for querying in the next frame
		for (auto& [key, state] : PlatformInput::s_KeyStates)
		{
			if (PlatformInput::s_KeyStatesForQuery[key].IsDown == KeyStateInfoForQuery::STATE_UP && state.IsPressed)
			{
				PlatformInput::s_KeyStatesForQuery[key].IsDown = KeyStateInfoForQuery::STATE_UP_TO_DOWN;
			}
			else if (PlatformInput::s_KeyStatesForQuery[key].IsDown == KeyStateInfoForQuery::STATE_UP_TO_DOWN && state.IsPressed)
			{
				PlatformInput::s_KeyStatesForQuery[key].IsDown = KeyStateInfoForQuery::STATE_DOWN;
			}
			else if (PlatformInput::s_KeyStatesForQuery[key].IsDown == KeyStateInfoForQuery::STATE_DOWN && state.IsPressed == false)
			{
				PlatformInput::s_KeyStatesForQuery[key].IsDown = KeyStateInfoForQuery::STATE_UP;
			}
			PlatformInput::s_KeyStatesForQuery[key].IsUp = PlatformInput::s_KeyStatesForQuery[key].IsPressed == true && state.IsPressed == false;
			if (PlatformInput::s_KeyStatesForQuery[key].IsLongDown == KeyStateInfoForQuery::STATE_UP && state.IsPressed && state.IsLongPressTriggered)
			{
				PlatformInput::s_KeyStatesForQuery[key].IsLongDown = KeyStateInfoForQuery::STATE_UP_TO_DOWN;
			}
			else if (PlatformInput::s_KeyStatesForQuery[key].IsLongDown == KeyStateInfoForQuery::STATE_UP_TO_DOWN && state.IsPressed && state.IsLongPressTriggered)
			{
				PlatformInput::s_KeyStatesForQuery[key].IsLongDown = KeyStateInfoForQuery::STATE_DOWN;
			}
			else if (PlatformInput::s_KeyStatesForQuery[key].IsLongDown == KeyStateInfoForQuery::STATE_DOWN && (state.IsPressed == false || !state.IsLongPressTriggered))
			{
				PlatformInput::s_KeyStatesForQuery[key].IsLongDown = KeyStateInfoForQuery::STATE_UP;
			}
			PlatformInput::s_KeyStatesForQuery[key].IsLongUp = PlatformInput::s_KeyStatesForQuery[key].IsLongPressed == true && (state.IsPressed == false || !state.IsLongPressTriggered);


			PlatformInput::s_KeyStatesForQuery[key].IsPressed = state.IsPressed;
			PlatformInput::s_KeyStatesForQuery[key].IsLongPressed = state.IsPressed && state.IsLongPressTriggered;
		}
		for (auto& [button, state] : PlatformInput::s_MouseBtnStates)
		{
			if(PlatformInput::s_MouseBtnStatesForQuery[button].IsDown == MouseBtnStateInfoForQuery::STATE_UP && state.IsPressed)
			{
				PlatformInput::s_MouseBtnStatesForQuery[button].IsDown = MouseBtnStateInfoForQuery::STATE_UP_TO_DOWN;
			}
			else if(PlatformInput::s_MouseBtnStatesForQuery[button].IsDown == MouseBtnStateInfoForQuery::STATE_UP_TO_DOWN && state.IsPressed)
			{
				PlatformInput::s_MouseBtnStatesForQuery[button].IsDown = MouseBtnStateInfoForQuery::STATE_DOWN;
			}
			else if(PlatformInput::s_MouseBtnStatesForQuery[button].IsDown == MouseBtnStateInfoForQuery::STATE_DOWN && state.IsPressed == false)
			{
				PlatformInput::s_MouseBtnStatesForQuery[button].IsDown = MouseBtnStateInfoForQuery::STATE_UP;
			}
			PlatformInput::s_MouseBtnStatesForQuery[button].IsUp = PlatformInput::s_MouseBtnStatesForQuery[button].IsPressed == true && state.IsPressed == false;
			if (PlatformInput::s_MouseBtnStatesForQuery[button].IsLongDown == MouseBtnStateInfoForQuery::STATE_UP && state.IsPressed && state.IsLongPressTriggered)
			{
				PlatformInput::s_MouseBtnStatesForQuery[button].IsLongDown = MouseBtnStateInfoForQuery::STATE_UP_TO_DOWN;
			}
			else if (PlatformInput::s_MouseBtnStatesForQuery[button].IsLongDown == MouseBtnStateInfoForQuery::STATE_UP_TO_DOWN && state.IsPressed && state.IsLongPressTriggered)
			{
				PlatformInput::s_MouseBtnStatesForQuery[button].IsLongDown = MouseBtnStateInfoForQuery::STATE_DOWN;
			}
			else if (PlatformInput::s_MouseBtnStatesForQuery[button].IsLongDown == MouseBtnStateInfoForQuery::STATE_DOWN && (state.IsPressed == false || !state.IsLongPressTriggered))
			{
				PlatformInput::s_MouseBtnStatesForQuery[button].IsLongDown = MouseBtnStateInfoForQuery::STATE_UP;
			}
			PlatformInput::s_MouseBtnStatesForQuery[button].IsLongUp = PlatformInput::s_MouseBtnStatesForQuery[button].IsLongPressed == true && (state.IsPressed == false || !state.IsLongPressTriggered);

			PlatformInput::s_MouseBtnStatesForQuery[button].IsPressed = state.IsPressed;
			PlatformInput::s_MouseBtnStatesForQuery[button].IsLongPressed = state.IsPressed && state.IsLongPressTriggered;
		}
	}

	void GLFWWindow::OnUpdate()
	{
		glfwPollEvents();
		m_Context->SwapBuffers();
		MouseButtonPreessedCallback();
		
	}

	void GLFWWindow::OnEndFrame()
	{
		UpdateKeyAndMouseStatesForQuery();
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
		if (GraphicsAPI::GetAPI() == GRAPHICS_API_OPENGL)
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
		return (float)glfwGetTime() * 1000;
	}
}