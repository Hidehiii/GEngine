#pragma once

#ifdef GE_PLATFORM_WINDOWS && GE_ENTRY_POINT

	extern GEngine::Application* GEngine::CreateApplication(ApplicationCommandLineArgs args);

	int main(int argc, char** argv)
	{
		GEngine::Log::Init();

		GE_PROFILE_BEGIN_SESSION("StartUp", "GEngineProfile-StartUp.json");
		auto app = GEngine::CreateApplication({ argc, argv});
		GE_PROFILE_END_SESSION();

		GE_PROFILE_BEGIN_SESSION("RunTime", "GEngineProfile-RunTime.json");
		app->Run();
		GE_PROFILE_END_SESSION();

		GE_PROFILE_BEGIN_SESSION("ShutDown", "GEngineProfile-ShutDown.json");
		delete(app);
		GE_PROFILE_END_SESSION();
	}

#endif