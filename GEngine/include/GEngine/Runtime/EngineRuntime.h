#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Core/Log.h"
#include "GEngine/LayerStack.h"
#include "GEngine/Math/Math.h"

#include <filesystem>

namespace GEngine
{
	class Config;
	class Window;
	class ImGuiLayer;
	class RenderSystem;
	class Event;
	class WindowCloseEvent;
	class WindowResizeEvent;

	struct RuntimeCommandLineArgs
	{
		int argc = 0;
		char** argv = nullptr;
		const char* operator[](int index) const { GE_CORE_ASSERT(index < argc, ""); return argv[index]; }
	};

	struct RuntimeSpecification
	{
		std::string Name = "GEngine";
		Vector2 Size = { 1920.0f, 1080.0f };
		RuntimeCommandLineArgs CommandLineArgs;
		std::filesystem::path WorkingDirectory;
		bool EnableScripting = true;
		bool EnablePhysics = true;
		bool EnableOpenSSL = true;
		bool EnableImGui = true;
	};

	class GENGINE_API EngineRuntime
	{
	public:
		explicit EngineRuntime(const RuntimeSpecification& specification);
		~EngineRuntime();

		void Run();
		void Close();
		void OnEvent(Event& event);
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		Window& GetWindow();
		const Ref<Config>& GetConfig() const { return m_Config; }
		ImGuiLayer* GetImGuiLayer() const { return m_ImGuiLayer; }
		const RuntimeSpecification& GetSpecification() const { return m_Specification; }

	private:
		void RunFrame();
		bool OnWindowClose(WindowCloseEvent& event);
		bool OnWindowResize(WindowResizeEvent& event);

	private:
		RuntimeSpecification m_Specification;
		Scope<Window> m_Window;
		Ref<Config> m_Config;
		Scope<RenderSystem> m_RenderSystem;
		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer = nullptr;
		std::string m_ConfigPath = "Config.ini";
		bool m_Running = true;
		bool m_Minimized = false;
	};
}
