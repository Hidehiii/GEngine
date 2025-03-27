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

		inline unsigned int GetWidth() const override { return m_Data.Width; }
		inline unsigned int GetHeight() const override { return m_Data.Height; }
		float GetTime() const override;

		virtual std::vector<const char*> GetRequiredExtensions() override;

		// Window attributes
		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }

		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		virtual GraphicsContext* GetContext() const override { return m_Context; }

		inline virtual void* GetNativeWindow() const override { return nullptr; }
	private:
		//GLFWwindow* m_Window;
		GraphicsContext* m_Context;

		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};
}


