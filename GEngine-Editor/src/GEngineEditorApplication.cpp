#include <GEngine.h>
#include <GEngine/EntryPoint.h> // Entry Point
#include "GEngineEditorLayer.h"


namespace GEngine
{
	class GEngineEditor : public Application
	{
	public:
		GEngineEditor(const ApplicationSpecification& spec)
			: Application(spec)
		{
			PushLayer(new GEngineEditorLayer());
		}

		~GEngineEditor()
		{

		}
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		Renderer::SetRenererAPI(RendererAPI::API::OpenGL);

		ApplicationSpecification spec;
		spec.Name = "GEngine Editor";
		spec.Size = { 2560, 1920 };
		spec.CommandLineArgs = args;
		return new GEngineEditor(spec);
	}
}

