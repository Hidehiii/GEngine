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

		ApplicationSpecification	spec;
		spec.Name					= "GEngine Editor";
		spec.Size					= { 1920, 1080 };
		spec.CommandLineArgs		= args;
		return new GEngineEditor(spec);
	}
}

