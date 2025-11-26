#include "GEpch.h"
#include "Application.h"
#include "Graphics/Graphics.h"
#include "GEngine/Scripting/ScriptEngine.h"
#include "tools/Serializer.h"
#include "Utils/GUIUtils.h"
#include "GEngine/Physics/3D/Physics3D.h"


namespace GEngine
{
	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& spec)
	{
		
		Log::Init();
		GE_CORE_ASSERT(!s_Instance, "Application already exists!");
		
		s_Instance		= this;
		m_Specification = spec;

		if (m_Specification.WorkingDirectory.empty() == false)
		{
			std::filesystem::current_path(m_Specification.WorkingDirectory);
		}

		m_Config = Ref<Config>(new Config());
		Serializer::Deserialize(m_ConfigPath, m_Config);

		GraphicsSpecification					graphicsSpec{};
		graphicsSpec.API						= (Graphics_API)m_Config->m_GraphicsAPI;
		graphicsSpec.CommandBufferCount			= m_Config->m_CommandBufferCount;
		graphicsSpec.DynamicUniformCount		= m_Config->m_DynamicUniformCount;
		graphicsSpec.FramesInFlight				= m_Config->m_FramesInFlight;
		graphicsSpec.ViewportWidth				= (uint32_t)spec.Size.x;
		graphicsSpec.ViewportHeight				= (uint32_t)spec.Size.y;

		Graphics::Setup(graphicsSpec);

		m_Window = Scope<Window>(Window::Create(WindowProps(m_Specification.Name, (uint32_t)m_Specification.Size.x, (uint32_t)m_Specification.Size.y)));
		m_Window->SetEventCallback(GE_BIND_CLASS_FUNCTION_LAMBDA(Application::OnEvent));
		m_Window->SetVSync(m_Config->m_VSync);

		m_GraphicsPresent = GraphicsPresent::Create();


		Graphics::Init();
		ScriptEngine::Init();
		Physics3D::Init();


		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		Serializer::Serialize(m_ConfigPath, m_Config);

		ScriptEngine::Shutdown();
		Physics3D::Shutdown();
	}

	void Application::Run()
	{
		while (m_Running) {
			Time::SetDeltaTime(m_Window->GetTime() - Time::GetRunTime());
			Time::SetRunTime(m_Window->GetTime());
			if (m_Minimized == false)
			{

				for (auto layer : m_LayerStack) {
					layer->OnUpdate();
				}
				if (m_GraphicsPresent->AquireImage())
				{
					
					for (auto layer : m_LayerStack) {
						layer->OnRender();
					}
					
					m_ImGuiLayer->Begin();
					{

						for (auto layer : m_LayerStack) {
							layer->OnImGuiRender();
						}
					}
					m_ImGuiLayer->End();
					
					m_GraphicsPresent->Begin();
					for (auto layer : m_LayerStack) {
						layer->OnPresent();
					}
					m_GraphicsPresent->End();
					
				}
				
				for (auto layer : m_LayerStack) {
					layer->OnLateUpdate();
				}
				
				for (auto layer : m_LayerStack) {
					layer->OnEndFrame();
				}
			}
			m_Window->OnUpdate();
			Graphics::FrameMove();
		}
	}
	void Application::Close()
	{
		m_Running = false;
		
	}
	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(GE_BIND_CLASS_FUNCTION_LAMBDA(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(GE_BIND_CLASS_FUNCTION_LAMBDA(Application::OnWindowResize));

		for(auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
			{
				break;
			}
		}
	}
	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}
	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}
		m_Minimized = false;
		Graphics::SetViewport(e.GetWidth(), e.GetHeight());
		return false;
	}
	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}
	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushOverlay(overlay);
	}
}