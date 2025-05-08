#include <GEngine.h>
#include <GEngine/EntryPoint.h> // Entry Point
#include "GpuDriven.h"
class DeferredRenderExample : public GEngine::Application
{
public:
	DeferredRenderExample(const GEngine::ApplicationSpecification& spec)
		: Application(spec)
	{
		PushLayer(new GEngine::GpuDriven());
	}

	~DeferredRenderExample()
	{

	}
};

GEngine::Application* GEngine::CreateApplication(ApplicationCommandLineArgs args)
{
	ApplicationSpecification spec;
	spec.Name = "Gpu Driven Example";
	spec.Size = { 1080, 720 };
	spec.CommandLineArgs = args;
	return new DeferredRenderExample(spec);
}