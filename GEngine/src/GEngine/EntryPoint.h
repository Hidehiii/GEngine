#pragma once

#ifdef GE_PLATFORM_WINDOWS && GE_ENTRY_POINT

	extern GEngine::Application* GEngine::CreateApplication(ApplicationCommandLineArgs args);

	int main(int argc, char** argv)
	{
		auto app = GEngine::CreateApplication({ argc, argv});

		app->Run();

		delete(app);
	}

#endif