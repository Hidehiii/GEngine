#include <GEngine.h>
#include <GEngine/EntryPoint.h>

#include "GameWorldLayer.h"

namespace GEngine
{
	class GameWorldApplication final : public Application
	{
	public:
		explicit GameWorldApplication(const ApplicationSpecification& specification)
			: Application(specification)
		{
			PushLayer(new GameWorldLayer());
		}
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationSpecification specification;
		specification.Name = "GEngine GameWorld";
		specification.Size = { 1280.0f, 720.0f };
		specification.CommandLineArgs = args;
		specification.EnableScripting = true;
		specification.EnablePhysics = false;
		specification.EnableOpenSSL = false;
		specification.EnableImGui = false;
#ifndef GE_SCRIPT_RUNTIME_ROOT
#define GE_SCRIPT_RUNTIME_ROOT "mono/lib"
#endif
#ifndef GE_SCRIPT_CORE_ASSEMBLY
#define GE_SCRIPT_CORE_ASSEMBLY "Resources/Scripts/GEngine-ScriptCore.dll"
#endif
		specification.ScriptRuntimeRoot = GE_SCRIPT_RUNTIME_ROOT;
		specification.ScriptCoreAssembly = GE_SCRIPT_CORE_ASSEMBLY;
		return new GameWorldApplication(specification);
	}
}
