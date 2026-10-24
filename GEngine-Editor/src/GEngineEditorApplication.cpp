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

		ApplicationSpecification	spec;
		spec.Name					= "GEngine Editor";
		spec.Size					= { 1920, 1080 };
		spec.CommandLineArgs		= args;
		spec.EnableScripting = std::filesystem::exists("Resources/Scripts/GEngine-ScriptCore.dll") &&
			std::filesystem::exists("mono/lib/mono/4.5/mscorlib.dll");
		return new GEngineEditor(spec);
	}
}

