#include <GEngine.h>
#include <GEngine/EntryPoint.h>

#include "RenderPipelineLayer.h"

namespace GEngine
{
	class RenderPipelineApplication final : public Application
	{
	public:
		explicit RenderPipelineApplication(const ApplicationSpecification& specification)
			: Application(specification)
		{
			PushLayer(new RenderPipelineLayer());
		}
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationSpecification specification;
		specification.Name = "GEngine Render Pipeline";
		specification.Size = {1280.0f, 720.0f};
		specification.CommandLineArgs = args;
		specification.EnableScripting = false;
		specification.EnablePhysics = false;
		specification.EnableOpenSSL = false;
		specification.EnableImGui = false;
		return new RenderPipelineApplication(specification);
	}
}
