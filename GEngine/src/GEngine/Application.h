#pragma once


#include "GEngine/Core/Core.h"
#include "Window.h"
#include "LayerStack.h"
#include "ImGui/ImGuiLayer.h"
#include "GEngine/Core/Time.h"
#include "Renderer/RenderCommand.h"
#include "GEngine/Math/Math.h"


namespace GEngine	
{
	class GENGINE_API Application
	{
	public:
		Application(const std::string& name = "GEngine", const Vector2& size = {1920.0f, 1080.0f});
		virtual ~Application();

		void Run();

		void Close();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }
	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
	private:
		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;
	private:
		static Application* s_Instance;
	};

	//To be defined in CLIENT
	Application* CreateApplication();
}


