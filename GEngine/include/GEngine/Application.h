#pragma once


#include "GEngine/Core/Core.h"
#include "GEngine/Core/Config.h"
#include "GEngine/Runtime/EngineRuntime.h"
#include "GEngine/Window.h"

int main(int argc, char** argv);

namespace GEngine
{

	using ApplicationCommandLineArgs = RuntimeCommandLineArgs;
	using ApplicationSpecification = RuntimeSpecification;

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

		inline ImGuiLayer* GetImGuiLayer() { return m_Runtime->GetImGuiLayer(); }

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return m_Runtime->GetWindow(); }
		inline const Ref<Config>& GetConfig() const { return m_Runtime->GetConfig(); }

		inline const ApplicationSpecification& GetSpecification() const { return m_Runtime->GetSpecification(); }
	private:
		Scope<EngineRuntime>			m_Runtime;
	private:
		static Application* s_Instance;
	};

	//To be defined in CLIENT
	Application* CreateApplication(ApplicationCommandLineArgs args);
}
