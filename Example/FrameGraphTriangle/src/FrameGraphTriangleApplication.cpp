#include <GEngine.h>
#include <GEngine/EntryPoint.h>

#include "FrameGraphTriangleLayer.h"

namespace GEngine
{
	class FrameGraphTriangleApplication final : public Application
	{
	public:
		explicit FrameGraphTriangleApplication(const ApplicationSpecification& specification)
			: Application(specification)
		{
			PushLayer(new FrameGraphTriangleLayer());
		}
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationSpecification specification;
		specification.Name = "GEngine Frame Graph Triangle";
		specification.Size = { 1280.0f, 720.0f };
		specification.CommandLineArgs = args;
		specification.EnableScripting = false;
		specification.EnablePhysics = false;
		specification.EnableOpenSSL = false;
		specification.EnableImGui = false;
		return new FrameGraphTriangleApplication(specification);
	}
}
