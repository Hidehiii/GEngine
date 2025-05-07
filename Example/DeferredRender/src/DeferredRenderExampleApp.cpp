#include <GEngine.h>
#include <GEngine/EntryPoint.h> // Entry Point
#include "DeferredRender.h"

using namespace GEngine;
class DeferredRenderExample : public GEngine::Application
{
public:
	DeferredRenderExample(const GEngine::ApplicationSpecification& spec)
		: Application(spec)
	{
		PushLayer(new DeferredRender);
	}

	~DeferredRenderExample()
	{

	}
};

GEngine::Application* GEngine::CreateApplication(ApplicationCommandLineArgs args)
{
	ApplicationSpecification spec;
	spec.Name = "Deferred Render Example";
	spec.Size = { 1080, 720 };
	spec.CommandLineArgs = args;
	return new DeferredRenderExample(spec);
}