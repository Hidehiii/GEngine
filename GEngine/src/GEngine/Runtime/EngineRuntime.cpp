#include "GEpch.h"
#include "GEngine/Runtime/EngineRuntime.h"

#include "GEngine/Core/Config.h"
#include "GEngine/Core/Input.h"
#include "GEngine/Core/Time.h"
#include "GEngine/Graphics/GraphicsAPI.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/ImGui/ImGuiLayer.h"
#include "GEngine/Physics/3D/Physics3D.h"
#include "GEngine/Renderer/RenderSystem.h"
#include "GEngine/Scripting/ScriptEngine.h"
#include "GEngine/Tools/OpenSSLTool.h"
#include "GEngine/Tools/Serializer.h"
#include "GEngine/Window.h"
#include "GEngine/Events/ApplicationEvent.h"

namespace GEngine
{
	EngineRuntime::EngineRuntime(const RuntimeSpecification& specification)
		: m_Specification(specification)
	{
		Log::Init();
		if (!m_Specification.WorkingDirectory.empty())
			std::filesystem::current_path(m_Specification.WorkingDirectory);

		m_Config = CreateRef<Config>();
		Serializer::Deserialize(m_ConfigPath, m_Config);

		GraphicsSpecification graphicsSpecification{};
		graphicsSpecification.API = static_cast<Graphics_API>(m_Config->GetGraphicsAPI());
		graphicsSpecification.CommandBufferCount = m_Config->GetCommandBufferCount();
		graphicsSpecification.FramesInFlight = m_Config->GetFramesInFlight();
		graphicsSpecification.ViewportWidth = static_cast<uint32_t>(m_Specification.Size.x);
		graphicsSpecification.ViewportHeight = static_cast<uint32_t>(m_Specification.Size.y);
		graphicsSpecification.WindowManagerAPI = m_Config->GetWindowManagerAPI();

		m_RenderSystem = CreateScope<RenderSystem>();
		m_RenderSystem->Configure(graphicsSpecification);

		m_Window = Scope<Window>(Window::Create(WindowProps(m_Specification.Name,
			static_cast<uint32_t>(m_Specification.Size.x), static_cast<uint32_t>(m_Specification.Size.y)),
			m_Config->GetWindowManagerAPI()));
		m_Window->SetEventCallback(GE_BIND_CLASS_FUNCTION_LAMBDA(EngineRuntime::OnEvent));
		m_Window->SetVSync(m_Config->GetVSync());

		m_RenderSystem->Initialize();

		Input::Init();
		if (m_Specification.EnableScripting)
			ScriptEngine::Init();
		if (m_Specification.EnablePhysics)
			Physics3D::Init();
		if (m_Specification.EnableOpenSSL)
			OpenSSLTool::Init();

		if (m_Specification.EnableImGui)
		{
			m_ImGuiLayer = new ImGuiLayer();
			PushOverlay(m_ImGuiLayer);
		}
	}

	EngineRuntime::~EngineRuntime()
	{
		Serializer::Serialize(m_ConfigPath, m_Config);
		if (m_Specification.EnableScripting)
			ScriptEngine::Shutdown();
		if (m_Specification.EnablePhysics)
			Physics3D::Shutdown();

		m_LayerStack.Clear();
		m_ImGuiLayer = nullptr;
		if (m_RenderSystem)
		{
			m_RenderSystem->Shutdown();
			m_RenderSystem.reset();
		}
		m_Window.reset();
	}

	void EngineRuntime::Run()
	{
		while (m_Running)
			RunFrame();
	}

	void EngineRuntime::Close()
	{
		m_Running = false;
	}

	void EngineRuntime::OnEvent(Event& event)
	{
		EventDispatcher::Dispatch<WindowCloseEvent>(event, GE_BIND_CLASS_FUNCTION_LAMBDA(EngineRuntime::OnWindowClose));
		EventDispatcher::Dispatch<WindowResizeEvent>(event, GE_BIND_CLASS_FUNCTION_LAMBDA(EngineRuntime::OnWindowResize));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(event);
			if (event.Handled)
				break;
		}
	}

	void EngineRuntime::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void EngineRuntime::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushOverlay(overlay);
	}

	Window& EngineRuntime::GetWindow()
	{
		return *m_Window;
	}

	void EngineRuntime::RunFrame()
	{
		const float currentTime = m_Window->GetTime();
		Time::SetDeltaTime(currentTime - Time::GetRunTime());
		Time::SetRunTime(currentTime);

		if (!m_Minimized)
		{
			for (auto* layer : m_LayerStack)
				layer->OnUpdate();

			m_RenderSystem->RenderFrame(m_LayerStack, m_ImGuiLayer);

			for (auto* layer : m_LayerStack)
				layer->OnLateUpdate();
			for (auto* layer : m_LayerStack)
				layer->OnEndFrame();
		}

		m_Window->OnUpdate();
		m_Window->OnEndFrame();
		m_RenderSystem->AdvanceFrame();
	}

	bool EngineRuntime::OnWindowClose(WindowCloseEvent& event)
	{
		m_Running = false;
		return true;
	}

	bool EngineRuntime::OnWindowResize(WindowResizeEvent& event)
	{
		if (event.GetWidth() == 0 || event.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;
		m_RenderSystem->Resize(event.GetWidth(), event.GetHeight());
		return false;
	}
}
