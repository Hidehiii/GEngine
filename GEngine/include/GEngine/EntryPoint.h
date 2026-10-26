#pragma once
#ifndef GE_ENTRY_POINT_H
#define GE_ENTRY_POINT_H
#include "GEngine/Runtime/ResourcePack.h"
#include <iostream>
#ifndef GE_PACKAGED_GAME
#define GE_PACKAGED_GAME 0
#endif

	extern GEngine::Application* GEngine::CreateApplication(ApplicationCommandLineArgs args);

	int main(int argc, char** argv)
	{
		try { GEngine::PreparePackagedResources(GE_PACKAGED_GAME != 0); }
		catch (const std::exception& error) { std::cerr << "Game resource pack failed: " << error.what() << '\n'; return 1; }
		auto app = GEngine::CreateApplication({ argc, argv});

		app->Run();

		delete(app);

		return 0;
	}
#ifdef GE_PLATFORM_WINDOWS
	int wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd)
	{
		try { GEngine::PreparePackagedResources(GE_PACKAGED_GAME != 0); }
		catch (const std::exception& error) { std::cerr << "Game resource pack failed: " << error.what() << '\n'; return 1; }
		auto app = GEngine::CreateApplication({ __argc, __argv });
		app->Run();
		delete(app);
		return 0;
	}
#endif
#endif

