#pragma once

#include "GEngine/Window.h"
#include "GEngine/Graphics/GraphicsContext.h"
#include <GLFW/glfw3.h>

namespace GEngine
{
	class GLFWWindow : public Window
	{
	public:
		GLFWWindow(const WindowProps& props);
		virtual ~GLFWWindow();

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
	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();

		virtual void MouseButtonPreessedCallback();
	private:
		GLFWwindow* m_Window;
		GraphicsContext* m_Context;

		WindowData m_Data;

		
	};
}



