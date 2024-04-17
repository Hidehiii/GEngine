#include <GEngine.h>
#include <GEngine/EntryPoint.h> // Entry Point
#include "GEngineEditorLayer.h"


namespace GEngine
{
	class GEngineEditor : public Application
	{
	public:
		GEngineEditor()
			: Application("GEngine Editor")
		{
			PushLayer(new GEngineEditorLayer());
		}

		~GEngineEditor()
		{

		}
	};

	Application* CreateApplication()
	{
		Renderer::SetRenererAPI(RendererAPI::API::OpenGL);
		return new GEngineEditor();
	}
}

