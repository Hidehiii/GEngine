#include "GEpch.h"
#include "GEngine/Application.h"


namespace GEngine
{
	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& spec)
	{
		GE_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;
		m_Runtime = CreateScope<EngineRuntime>(spec);
	}

	Application::~Application()
	{
		m_Runtime.reset();
		s_Instance = nullptr;
	}

	void Application::Run()
	{
		m_Runtime->Run();
	}
	void Application::Close()
	{
		m_Runtime->Close();
	}
	void Application::OnEvent(Event& e)
	{
		m_Runtime->OnEvent(e);
	}
	void Application::PushLayer(Layer* layer)
	{
		m_Runtime->PushLayer(layer);
	}
	void Application::PushOverlay(Layer* overlay)
	{
		m_Runtime->PushOverlay(overlay);
	}
}
