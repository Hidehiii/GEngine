#include "GEpch.h"
#include "Win32Window.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/D3D12/D3D12Context.h"

namespace GEngine
{
	LPCWSTR StringToLPCWSTR(const std::string& str)
	{
		size_t	strSize = str.length() + 1;
		size_t convertedChars = 0;
		wchar_t* wcstring = (wchar_t*)malloc(sizeof(wchar_t) * (str.length() - 1));
		mbstowcs_s(&convertedChars, wcstring, strSize, str.c_str(), _TRUNCATE);

		return wcstring;
	}

	LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_CREATE:
			if (lParam)
			{
				auto params = reinterpret_cast<LPCREATESTRUCTW>(lParam);
				SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(params->lpCreateParams));
			}
			break;
		case WM_ACTIVATEAPP:

		case WM_ACTIVATE:

		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
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
			if (wParam == SIZE_MINIMIZED)
			{

			}
			break;
		case WM_ENTERSIZEMOVE:

		case WM_EXITSIZEMOVE:

		case WM_GETMINMAXINFO:
			if (lParam)
			{
				auto info = reinterpret_cast<MINMAXINFO*>(lParam);
				info->ptMinTrackSize.x = 320;
				info->ptMinTrackSize.y = 200;
			}
			break;

		case WM_POWERBROADCAST:
			switch (wParam)
			{
			case PBT_APMQUERYSUSPEND:
				
				return TRUE;

			case PBT_APMRESUMESUSPEND:
				
				return TRUE;
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_INPUT:
		case WM_MOUSEHOVER:

		case WM_KEYDOWN:
		case WM_KEYUP:
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

		case WM_MOUSEACTIVATE :
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
		m_Window = CreateWindow(L"MainWnd", StringToLPCWSTR(props.Title), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, hInstance, 0);
		if (!(m_Window))
		{
			MessageBox(0, L"CreateWindow Failed", 0, 0);
			GE_CORE_ASSERT(false, "");
		}

		ShowWindow(m_Window, SW_SHOW);
		UpdateWindow(m_Window);

		if (Graphics::GetGraphicsAPI() == GraphicsAPI::API::Direct3DX12)
		{
			m_Context = new D3D12Context(m_Window);
		}

		m_Context->Init(m_Data.Width, m_Data.Height);
	}
	Win32Window::~Win32Window()
	{
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
	float Win32Window::GetTime() const
	{
		return 0.0f;
	}
	std::vector<const char*> Win32Window::GetRequiredExtensions()
	{
		return std::vector<const char*>();
	}
	void Win32Window::SetVSync(bool enabled)
	{
	}
	bool Win32Window::IsVSync() const
	{
		return false;
	}
}