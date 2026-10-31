#pragma once

#include "GEngine/Core/Core.h"

#include <functional>
#include <memory>
#include <string>

namespace GEngine
{
	class GameObject;
	class NativeScriptBehaviour;
	struct RuntimeSpecification;

	class GENGINE_API ScriptEngine
	{
	public:

		static bool Init(const RuntimeSpecification& specification);
		static void Shutdown();
		static bool IsInitialized();
		static bool RegisterNativeScript(const std::string& className,
			std::function<std::unique_ptr<NativeScriptBehaviour>()> factory);
		static bool HasNativeScript(const std::string& className);

		static void OnGameObjectAwake(GameObject& object);
		static void OnGameObjectStart(GameObject& object);
		static void OnGameObjectEnable(GameObject& object);
		static void OnGameObjectDisable(GameObject& object);
		static void OnGameObjectFixedUpdate(GameObject& object, float fixedDeltaTime);
		static void OnGameObjectUpdate(GameObject& object, float deltaTime);
		static void OnGameObjectLateUpdate(GameObject& object);
		static void OnGameObjectPause(GameObject& object);
		static void OnGameObjectResume(GameObject& object);
		static void OnGameObjectDestroy(GameObject& object);
		static void RemoveScript(GameObject& object, const std::string& className);
	private:
		static bool InitMono(const RuntimeSpecification& specification);
		static void ShutdownMono();
	};
}


