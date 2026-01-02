#include <GEngine.h>
#include <GEngine/EntryPoint.h> // Entry Point
#include "InputSystemExplame.h"

using namespace GEngine;

class InputSystemExplameApp : public GEngine::Application
{
public:
	InputSystemExplameApp(const GEngine::ApplicationSpecification& spec)
		: Application(spec)
	{
		PushLayer(new InputSystemExplame);
	}

	~InputSystemExplameApp()
	{
	}
};

GEngine::Application* GEngine::CreateApplication(ApplicationCommandLineArgs args)
{
	ApplicationSpecification spec;
	spec.Name = "Shader Compile Example";
	spec.Size = { 1080, 720 };
	spec.CommandLineArgs = args;
	return new InputSystemExplameApp(spec);
}