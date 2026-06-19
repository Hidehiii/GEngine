#include "GEpch.h"
#include "Win32Window.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/D3D12/D3D12Context.h"
#include "GEngine/Tools/StringHelper.h"
#include "Surface/Win32/Win32Input.h"
#include "GEngine/Application.h"

namespace GEngine
{

	LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		Win32Window* w = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		if (!w)
			return DefWindowProc(hwnd, msg, wParam, lParam);
		return w->WndProc(hwnd, msg, wParam, lParam);
	}

	Win32Window::Win32Window(const WindowProps& props)
	{
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

#if defined(DEBUG) | defined(_DEBUG) | defined(GE_DEBUG)
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
		HINSTANCE hInstance = GetModuleHandle(0);

		WNDCLASS			wc;
		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= MainWndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= hInstance;
		wc.hIcon			= LoadIcon(0, IDI_APPLICATION);
		wc.hCursor			= LoadCursor(0, IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)GetStockObject(NULL_BRUSH);
		wc.lpszMenuName		= 0;
		wc.lpszClassName	= L"MainWnd";
		if (RegisterClass(&wc) == false)
		{
			MessageBox(0, L"RegisterClass Failed", 0, 0);
			GE_CORE_ASSERT(false, "");
		}
		RECT R = { 0, 0, props.Width, props.Height };
		AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
		int width = R.right - R.left;
		int height = R.bottom - R.top;
		m_Window = CreateWindow(L"MainWnd", StringHelper::StringToWideString(props.Title).c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, hInstance, 0);
		if (!(m_Window))
		{
			MessageBox(0, L"CreateWindow Failed", 0, 0);
			GE_CORE_ASSERT(false, "");
		}

		// init time counter
		QueryPerformanceFrequency(&m_Frequency);
		QueryPerformanceCounter(&m_StartCounter);

		ShowWindow(m_Window, SW_SHOW);
		UpdateWindow(m_Window);

		GE_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_OPENGL:
		{
			GE_CORE_ASSERT(false, "OpenGL is not supported on Win32 platform!");
			break;
		}
		case GRAPHICS_API_VULKAN:
		{
			GE_CORE_ASSERT(false, "Vulkan is not supported on Win32 platform!");
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

		SetWindowLongPtr(m_Window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
		m_Context->Init(m_Data.Width, m_Data.Height);
	}
	Win32Window::~Win32Window()
	{
		m_Context->Uninit();
	}
	void Win32Window::OnUpdate()
	{
		if (m_Message.message == WM_QUIT)
		{
			GE_CORE_INFO("Win32 window quit");
			return;
		}
		if (PeekMessage(&m_Message, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&m_Message);
			DispatchMessage(&m_Message);
		}
		else
		{

		}
	}
	void Win32Window::OnEndFrame()
	{
	}
	float Win32Window::GetTime() const
	{
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		double deltaSec = static_cast<double>(now.QuadPart - m_StartCounter.QuadPart) / static_cast<double>(m_Frequency.QuadPart);
		return static_cast<float>(deltaSec) * 1000.0f;
	}
	std::vector<const char*> Win32Window::GetRequiredExtensions()
	{
		return std::vector<const char*>();
	}
	void Win32Window::SetVSync(bool enabled)
	{
		m_Data.VSync = enabled;
		m_Context->SetVSync(enabled);
	}
	bool Win32Window::IsVSync() const
	{
		return m_Data.VSync;
	}
	LRESULT Win32Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_CREATE:
		{
			if (lParam)
			{
				auto params = reinterpret_cast<LPCREATESTRUCTW>(lParam);
				SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(params->lpCreateParams));
			}
			break;
		}
		case WM_ACTIVATEAPP:

		case WM_ACTIVATE:

		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		{
			MouseCode btnCode = MOUSE_BUTTON_LEFT;
			MouseBtnStateInfo& btnState = PlatformInput::s_MouseBtnStates[btnCode];

			MouseButtonDownEvent downEvent(btnCode);
			m_Data.EventCallback(downEvent);

			MouseButtonPressedEvent pressedEvent(btnCode);
			m_Data.EventCallback(pressedEvent);

			btnState.IsPressed = true;
			btnState.PressStartTime = int(GetTime());
			int x, y = 0;
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			btnState.PressX = (float)x;
			btnState.PressY = (float)y;
			btnState.IsLongPressTriggered = false;

			// handling double clicked
			auto& lastBtn = PlatformInput::s_LastMouseBtnClicked;
			if (lastBtn.first == btnCode && int(GetTime()) - lastBtn.second <= Application::Get().GetConfig()->GetDoubleClickThresholdMs())
			{
				MouseButtonDoubleClickEvent doubleClickEvent(btnCode);
				m_Data.EventCallback(doubleClickEvent);
				// reset last clicked button to prevent triple click being detected as double click
				lastBtn = { MouseCode::MOUSE_BUTTON_UNKNOWN, 0 };
			}
			else
			{
				lastBtn = { btnCode, int(GetTime()) };
			}

			break;
		}
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:

		case WM_PAINT:

		case WM_MOVE:

		case WM_SIZE:
		{
			if (wParam == SIZE_MINIMIZED)
			{

			}
			break;
		}
		case WM_ENTERSIZEMOVE:

		case WM_EXITSIZEMOVE:

		case WM_GETMINMAXINFO:
		{
			if (lParam)
			{
				auto info = reinterpret_cast<MINMAXINFO*>(lParam);
				info->ptMinTrackSize.x = 320;
				info->ptMinTrackSize.y = 200;
			}
			break;
		}

		case WM_POWERBROADCAST:
		{
			switch (wParam)
			{
			case PBT_APMQUERYSUSPEND:

				return TRUE;

			case PBT_APMRESUMESUSPEND:

				return TRUE;
			}
			break;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		case WM_INPUT:
		case WM_MOUSEHOVER:

		case WM_KEYDOWN:
		{
			KeyCode keycode = Win32ToKeyCode(static_cast<uint32_t>(wParam));
			KeyStateInfo& keyState = PlatformInput::s_KeyStates[keycode];

			BOOL isFirstDown = (lParam & (1 << 30)) == 0;
			if (isFirstDown)
			{
				KeyDownEvent downEvent(keycode);
				m_Data.EventCallback(downEvent);

				keyState.IsPressed = true;
				keyState.PressStartTime = int(GetTime());
				keyState.IsLongPressTriggered = false;
			}

			if (keyState.IsPressed == false)
			{
				// ignore auto-repeat keydown messages if the key is not already marked as pressed
				break;
			}

			KeyPressedEvent pressedEvent(keycode);
			m_Data.EventCallback(pressedEvent);

			if (!keyState.IsLongPressTriggered)
			{
				if (Application::Get().GetConfig()->GetLongPressThresholdMs() <= int(GetTime()) - keyState.PressStartTime)
				{
					KeyLongDownEvent longDownEvent(keycode);
					m_Data.EventCallback(longDownEvent);
					// 触发长按事件
					KeyLongPressedEvent longPressEvent(keycode);
					m_Data.EventCallback(longPressEvent);

					keyState.IsLongPressTriggered = true;
				}
			}
			else
			{
				KeyLongPressedEvent longPressEvent(keycode);
				m_Data.EventCallback(longPressEvent);
			}
			break;
		}
		case WM_KEYUP:
		{
			KeyCode keycode = Win32ToKeyCode(static_cast<uint32_t>(wParam));
			KeyStateInfo& keyState = PlatformInput::s_KeyStates[keycode];

			KeyUpEvent upEvent(keycode);
			m_Data.EventCallback(upEvent);

			if (keyState.IsPressed)
			{
				if (keyState.IsLongPressTriggered)
				{
					// long pressed has triggered before, trigger long up event
					KeyLongUpEvent longUpEvent(keycode);
					m_Data.EventCallback(longUpEvent);
				}
				else
				{
					// normal key up event
					KeyClickEvent clickEvent(keycode);
					m_Data.EventCallback(clickEvent);
				}

				keyState.IsPressed = false;
				keyState.IsLongPressTriggered = false;
			}
			break;
		}
		case WM_SYSKEYUP:

		case WM_SYSKEYDOWN:
			//if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
			//{
			//	// Implements the classic ALT+ENTER fullscreen toggle
			//	if (s_fullscreen)
			//	{
			//		SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
			//		SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);

			//		int width = 800;
			//		int height = 600;
			//		if (sample)
			//			sample->GetDefaultSize(width, height);

			//		ShowWindow(hWnd, SW_SHOWNORMAL);

			//		SetWindowPos(hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
			//	}
			//	else
			//	{
			//		SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP);
			//		SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_TOPMOST);

			//		SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

			//		ShowWindow(hWnd, SW_SHOWMAXIMIZED);
			//	}

			//	s_fullscreen = !s_fullscreen;
			//}
			//Keyboard::ProcessMessage(message, wParam, lParam);
			break;

		case WM_MOUSEACTIVATE:
			// When you click activate the window, we want Mouse to ignore that event.
			return MA_ACTIVATEANDEAT;

		case WM_MENUCHAR:
			// A menu is active and the user presses a key that does not correspond
			// to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
			return MAKELRESULT(0, MNC_CLOSE);
		default:
			break;
		}

		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}