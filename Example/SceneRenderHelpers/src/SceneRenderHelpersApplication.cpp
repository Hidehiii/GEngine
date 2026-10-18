#include <GEngine.h>
#include <GEngine/EntryPoint.h>

#include "SceneRenderHelpersLayer.h"

namespace GEngine
{
	class SceneRenderHelpersApplication final : public Application
	{
	public:
		explicit SceneRenderHelpersApplication(const ApplicationSpecification& specification)
			: Application(specification)
		{
			PushLayer(new SceneRenderHelpersLayer());
		}
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationSpecification specification;
		specification.Name = "GEngine Scene Render Helpers";
		specification.Size = { 1280.0f, 720.0f };
		specification.CommandLineArgs = args;
		specification.EnableScripting = false;
		specification.EnablePhysics = false;
		specification.EnableOpenSSL = false;
		specification.EnableImGui = false;
		return new SceneRenderHelpersApplication(specification);
	}
}
