#include <GEngine.h>
#include <GEngine/EntryPoint.h> // Entry Point
#include "Sandbox2D.h"

class ExplameLayer : public GEngine::Layer
{
public:
	ExplameLayer()
		: Layer("Example")
	{
		
		
	}

	void OnUpdate() override 
	{

		

	}

	void OnGuiRender() override
	{

	}

	void OnEvent(GEngine::Event& event) override
	{
		
	}

private:
	
};

class Sandbox : public GEngine::Application
{
public:
	Sandbox()
		: Application()
	{
		//PushLayer(new ExplameLayer());
		PushLayer(new Sandbox2D());
	}

	~Sandbox()
	{

	}
};

GEngine::Application* GEngine::CreateApplication()
{
	return new Sandbox();
}