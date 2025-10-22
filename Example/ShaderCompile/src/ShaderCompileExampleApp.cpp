#include <GEngine.h>
#include <GEngine/EntryPoint.h> // Entry Point
#include "ShaderCompile.h"

using namespace GEngine;

class ShaderCompileExample : public GEngine::Application
{
public:
	ShaderCompileExample(const GEngine::ApplicationSpecification& spec)
		: Application(spec)
	{
		PushLayer(new ShaderCompile);
	}

	~ShaderCompileExample()
	{
	}
};

GEngine::Application* GEngine::CreateApplication(ApplicationCommandLineArgs args)
{
	ApplicationSpecification spec;
	spec.Name = "Shader Compile Example";
	spec.Size = { 1080, 720 };
	spec.CommandLineArgs = args;
	return new ShaderCompileExample(spec);
}