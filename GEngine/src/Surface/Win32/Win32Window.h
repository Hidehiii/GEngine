#pragma once
#include "GEngine/Window.h"
#include "GEngine/Graphics/GraphicsContext.h"

namespace GEngine
{
	class Win32Window : public Window
	{
	public:
		Win32Window(const WindowProps& props);
		virtual ~Win32Window();

		void OnUpdate() override;
		void OnEndFrame() override;

		inline unsigned int GetWidth() const override { return m_Data.Width; }
		inline unsigned int GetHeight() const override { return m_Data.Height; }
		float GetTime() const override;

		virtual std::vector<const char*> GetRequiredExtensions() override;

		// Window attributes
		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }

		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		virtual GraphicsContext* GetContext() const override { return m_Context; }

		inline virtual void* GetNativeWindow() const override { return m_Window; }

		LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	private:
		virtual void MouseButtonDownCallback(WPARAM wParam, LPARAM lParam, MouseCode btnCode);
		virtual void MouseButtonPressedCallback();
		virtual void MouseButtonUpCallback(WPARAM wParam, LPARAM lParam, MouseCode btnCode);
	private:
		HWND m_Window;
		MSG m_Message = { 0 };
		LARGE_INTEGER m_Frequency;
		LARGE_INTEGER m_StartCounter;

		GraphicsContext* m_Context;

		WindowData m_Data;
	};
}


