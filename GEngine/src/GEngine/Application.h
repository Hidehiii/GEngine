#pragma once


#include "GEngine/Core/Core.h"
#include "Window.h"
#include "LayerStack.h"
#include "ImGui/ImGuiLayer.h"
#include "GEngine/Core/Time.h"
#include "Renderer/RenderCommand.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Renderer/GraphicsPresent.h"
#include "GEngine/Core/Config.h"
#include <filesystem>

int main(int argc, char** argv);

namespace GEngine	
{

	struct ApplicationCommandLineArgs
	{
		int argc = 0;
		char** argv = nullptr;
		const char* operator[](int index) const { GE_CORE_ASSERT(index < argc, ""); return argv[index]; }
	};
	struct ApplicationSpecification
	{
		std::string Name = "GEngine";
		Vector2 Size = { 1920.0f, 1080.0f};
		ApplicationCommandLineArgs CommandLineArgs;
		std::filesystem::path WorkingDirectory;
	};

	class GENGINE_API Application
	{
	public:
		Application(const ApplicationSpecification& spec);
		virtual ~Application();

		void Run();

		void Close();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }
		inline Ref<Config> GetConfig() { return m_Config; }

		inline const ApplicationSpecification& GetSpecification() const { return m_Specification; }
	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
	private:
		ApplicationSpecification	m_Specification;
		Scope<Window>				m_Window;
		Ref<Config>					m_Config;
		std::string					m_ConfigPath = "Config.ini";
		ImGuiLayer*					m_ImGuiLayer;
		bool						m_Running = true;
		bool						m_Minimized = false;
		LayerStack					m_LayerStack;
		Scope<GraphicsPresent>		m_GraphicsPresent;
	private:
		static Application* s_Instance;
	};

	//To be defined in CLIENT
	Application* CreateApplication(ApplicationCommandLineArgs args);
}


