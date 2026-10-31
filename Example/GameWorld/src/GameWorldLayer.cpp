#include "GameWorldLayer.h"
#include "FailingNativeScript.h"
#include "NativeCounterScript.h"
#include <GEngine/Physics/2D/Physics2D.h>
#include <GEngine/Physics/3D/Physics3D.h>
#include <filesystem>
#include <stdexcept>

namespace GEngine
{
	void GameWorldLayer::OnAttach()
	{
		auto scene = CreateRef<Scene>();
		auto normal = scene->CreateGameObject(0x1000000000000001ull, "Normal");
		auto native = scene->CreateGameObject(0x1000000000000004ull, "Native");
		auto failing = scene->CreateGameObject(0x1000000000000002ull, "Failing");
		auto failingNative = scene->CreateGameObject(0x1000000000000005ull, "FailingNative");
		auto inactive = scene->CreateGameObject(0x1000000000000003ull, "Inactive");
		normal.AddScript("GEngine.WorldLogicScript");
		native.AddScript("GameWorld.NativeCounterScript");
		failing.AddScript("GEngine.FailingScript");
		failingNative.AddScript("GameWorld.FailingNativeScript");
		inactive.AddScript("GEngine.WorldLogicScript");
		inactive.SetActive(false);

		const auto temporary = std::filesystem::temp_directory_path() /
			("gengine-gameworld-" + std::to_string(static_cast<uint64_t>(UUID())) + ".gscene");
		auto cleanup = [&]
		{
			std::error_code ignored;
			std::filesystem::remove(temporary, ignored);
		};
		try
		{
			Serializer::Serialize(temporary.u8string(), scene);
			Ref<Scene> restored = CreateRef<Scene>();
			Serializer::Deserialize(temporary.u8string(), restored);
			auto restoredNormal = restored->GetGameObjects<>().front();
			if (!restoredNormal.HasScript("GEngine.WorldLogicScript") ||
				restoredNormal.GetScriptClasses().size() != 1)
				throw std::runtime_error("Script class scene roundtrip failed.");
		}
		catch (...) { cleanup(); throw; }
		cleanup();

		scene->OnAwake();
		scene->OnStart();
		scene->OnUpdate(0.025f);
		scene->OnPause();
		scene->OnUpdate(0.025f);
		scene->OnResume();
		scene->OnUpdate(0.025f);
		normal.SetActive(false);
		normal.SetActive(true);
		scene->DestroyGameObject(normal);
		scene->DestroyGameObject(native);
		scene->DestroyGameObject(failingNative);
		scene->OnEndFrame();

		{
			Physics2DWorld physics2D({0.0f, -9.81f}, nullptr);
			physics2D.Simulate(1.0f / 60.0f);
		}
		Physics3D::Init();
		Physics3D::Shutdown();
		GE_INFO("Physics backend smoke passed: Box2D step and PhysX init/shutdown.");

		GE_INFO("GameWorld checks completed: C#/C++ lifecycle, fixed/variable update, pause, failure, disable/enable and destroy.");
		Application::Get().Close();
	}
}
