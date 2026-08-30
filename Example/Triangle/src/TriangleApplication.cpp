#include <GEngine.h>
#include <GEngine/EntryPoint.h>

#include "TriangleLayer.h"

namespace GEngine
{
	class TriangleApplication final : public Application
	{
	public:
		explicit TriangleApplication(const ApplicationSpecification& specification)
			: Application(specification)
		{
			PushLayer(new TriangleLayer());
		}
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationSpecification specification;
			specification.Name = "GEngine Triangle (D3D12)";
			specification.Size = { 1280.0f, 720.0f };
			specification.CommandLineArgs = args;
			specification.EnableScripting = false;
			specification.EnablePhysics = false;
			specification.EnableOpenSSL = false;
			specification.EnableImGui = false;
			return new TriangleApplication(specification);
	}
}
