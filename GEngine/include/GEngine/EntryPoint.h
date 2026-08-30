#pragma once
#ifndef GE_ENTRY_POINT_H
#define GE_ENTRY_POINT_H

	extern GEngine::Application* GEngine::CreateApplication(ApplicationCommandLineArgs args);

	int main(int argc, char** argv)
	{
		auto app = GEngine::CreateApplication({ argc, argv});

		app->Run();

		delete(app);

		return 0;
	}
#ifdef GE_PLATFORM_WINDOWS
	int wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd)
	{
		auto app = GEngine::CreateApplication({ __argc, __argv });
		app->Run();
		delete(app);
		return 0;
	}
#endif
#endif

