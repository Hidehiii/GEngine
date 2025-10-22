#include <GEngine.h>
#include <GEngine/EntryPoint.h> // Entry Point
#include "Sandbox2D.h"

class Sandbox : public GEngine::Application
{
public:
	Sandbox(const GEngine::ApplicationSpecification& spec)
		: Application(spec)
	{
		PushLayer(new Sandbox2D()); 
	}

	~Sandbox()
	{

	}
};

GEngine::Application* GEngine::CreateApplication(ApplicationCommandLineArgs args)
{
	ApplicationSpecification spec;
	spec.Name = "Sandbox";
	spec.Size = { 1080, 720 };
	spec.CommandLineArgs = args;
	return new Sandbox(spec);
}