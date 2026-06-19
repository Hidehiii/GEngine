#include "GEpch.h"
#include "GLFWWindow.h"
#include "GEngine/Graphics/GraphicsAPI.h"
#include "Platform/OpenGL/OpenGLContext.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/D3D12/D3D12Context.h"
#include "Surface/GLFW/GLFWInput.h"
#include <GLFW/glfw3.h>
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

		if (Graphics::GetGraphicsAPI() == GRAPHICS_API_VULKAN)
		{
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}
		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);

		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_OPENGL:
		{
			m_Context = new OpenGLContext(m_Window);
			break;
		}
		case GRAPHICS_API_VULKAN:
		{
			m_Context = new VulkanContext(m_Window);
			m_Context->SetRequiredExtensions(GetRequiredExtensions());
			break;
		}
		case GRAPHICS_API_DIRECT3DX12:
		{
			m_Context = new D3D12Context(m_Window);
			break;
		}
		default:
			GE_CORE_ASSERT(false, "Unknown Graphics API!");
			break;
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
				
				KeyStateInfo& keyState = Input::s_KeyStates[keycode];

				keyState.IsPressed = true;
				keyState.PressStartTime = int(glfwGetTime() * 1000);
				keyState.IsLongPressTriggered = false;
				break;
			}
			case GLFW_RELEASE:
			{
				KeyCode keycode = GLFWToKeyCode(key);
				KeyStateInfo& keyState = Input::s_KeyStates[keycode];

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
				KeyStateInfo& keyState = Input::s_KeyStates[keycode];

				if(keyState.IsPressed == false)
				{
					// 如果按键状态为未按下，忽略重复事件
					break;
				}
				KeyPressedEvent event(keycode);
				data.EventCallback(event);

				
				if(!keyState.IsLongPressTriggered)
				{
					if (Application::Get().GetConfig()->GetLongPressThresholdMs() <= int(glfwGetTime() * 1000) - keyState.PressStartTime)
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
				MouseCode buttonCode = GLFWToMouseCode(button);
				MouseBtnStateInfo& btnState = Input::s_MouseBtnStates[buttonCode];

				btnState.IsPressed = true;
				btnState.PressStartTime = int(glfwGetTime() * 1000);
				double x, y = 0;
				glfwGetCursorPos(window, &x, &y);
				btnState.PressX = (float)x;
				btnState.PressY = (float)y;
				btnState.IsLongPressTriggered = false;

				Input::s_MouseX = btnState.PressX;
				Input::s_MouseY = btnState.PressY;

				MouseButtonDownEvent downEvent(buttonCode, Input::s_MouseX, Input::s_MouseY);
				data.EventCallback(downEvent);

				MouseButtonPressedEvent event(buttonCode, Input::s_MouseX, Input::s_MouseY);
				data.EventCallback(event);

				

				// handling double clicked
				auto& lastBtn = Input::s_LastMouseBtnClicked;
				if(lastBtn.first == buttonCode && int(glfwGetTime() * 1000) - lastBtn.second <= Application::Get().GetConfig()->GetDoubleClickThresholdMs())
				{
					MouseButtonDoubleClickEvent doubleClickEvent(buttonCode, Input::s_MouseX, Input::s_MouseY);
					data.EventCallback(doubleClickEvent);
					// reset last clicked button to prevent triple click being detected as double click
					lastBtn = { MouseCode::MOUSE_BUTTON_UNKNOWN, 0 };
				}
				else
				{
					// record the last clicked button and timestamp for double click detection
					lastBtn = { buttonCode, (float)glfwGetTime() * 1000 };
				}
				
				break;
			}
			case GLFW_RELEASE:
			{
				MouseCode buttonCode = GLFWToMouseCode(button);
				MouseBtnStateInfo& btnState = Input::s_MouseBtnStates[buttonCode];

				double x, y = 0;
				glfwGetCursorPos(window, &x, &y);
				Input::s_MouseX = (float)x;
				Input::s_MouseY = (float)y;

				MouseButtonUpEvent event(buttonCode, Input::s_MouseX, Input::s_MouseY);
				data.EventCallback(event);

				if(btnState.IsPressed)
				{
					if(btnState.IsLongPressTriggered)
					{
						// 长按已触发，执行长按释放逻辑（如果有的话）
						MouseButtonLongUpEvent longUpEvent(buttonCode, Input::s_MouseX, Input::s_MouseY);
						data.EventCallback(longUpEvent);
					}
					else
					{
						// 普通按键释放逻辑
						MouseButtonClickEvent clickEvent(buttonCode, Input::s_MouseX, Input::s_MouseY);
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
		for(auto& [button, btnState] : Input::s_MouseBtnStates)
		{
			int btnCode = MouseCodeToGLFW(button);
			auto state = glfwGetMouseButton(m_Window, btnCode);

			double x, y = 0;
			glfwGetCursorPos(m_Window, &x, &y);
			Input::s_MouseX = (float)x;
			Input::s_MouseY = (float)y;

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
					MouseButtonPressedEvent event(button, Input::s_MouseX, Input::s_MouseY);
					WindowData& data = *(WindowData*)glfwGetWindowUserPointer(m_Window);
					data.EventCallback(event);

					if (!btnState.IsLongPressTriggered)
					{
						if (Application::Get().GetConfig()->GetLongPressThresholdMs() <= int(glfwGetTime() * 1000) - btnState.PressStartTime)
						{
							MouseButtonLongDownEvent longDownEvent(button, Input::s_MouseX, Input::s_MouseY);
							data.EventCallback(longDownEvent);
							// 触发长按事件
							MouseButtonLongPressedEvent longPressEvent(button, Input::s_MouseX, Input::s_MouseY);
							data.EventCallback(longPressEvent);
							btnState.IsLongPressTriggered = true;
						}
					}
					else
					{
						MouseButtonLongPressedEvent longPressEvent(button, Input::s_MouseX, Input::s_MouseY);
						data.EventCallback(longPressEvent);
					}
				}
			}
		}
	}

	void GLFWWindow::OnUpdate()
	{
		glfwPollEvents();
		if (Graphics::GetGraphicsAPI() == GRAPHICS_API_OPENGL)
		{
			glfwSwapBuffers(m_Window);
		}
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