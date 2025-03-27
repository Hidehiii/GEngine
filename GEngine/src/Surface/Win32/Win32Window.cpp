#include "GEpch.h"
#include "Win32Window.h"

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
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		default:
			break;
		}

		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	Win32Window::Win32Window(const WindowProps& props)
	{
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
		HWND mhMainWnd = CreateWindow(L"MainWnd", StringToLPCWSTR(props.Title), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, hInstance, 0);
		if (!mhMainWnd)
		{
			MessageBox(0, L"CreateWindow Failed", 0, 0);
			GE_CORE_ASSERT(false, "");
		}

		ShowWindow(mhMainWnd, SW_SHOW);
		UpdateWindow(mhMainWnd);

		MSG msg = { 0 };
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{

			}
		}
	}
	Win32Window::~Win32Window()
	{
	}
	void Win32Window::OnUpdate()
	{
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