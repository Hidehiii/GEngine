#pragma once

#include "GEngine/Object/GameObject.h"
#include "GEngine/Scripting/ScriptEngine.h"

#include <memory>
#include <string>
#include <utility>

namespace GEngine
{
	class NativeScriptBehaviour
	{
	public:
		virtual ~NativeScriptBehaviour() = default;

		virtual void OnAwake() {}
		virtual void OnStart() {}
		virtual void OnEnable() {}
		virtual void OnDisable() {}
		virtual void OnFixedUpdate(float fixedDeltaTime) {}
		virtual void OnUpdate(float deltaTime) {}
		virtual void OnLateUpdate() {}
		virtual void OnPause() {}
		virtual void OnResume() {}
		virtual void OnDestroy() {}

		GameObject GetGameObject() const { return m_GameObject; }
		void SetGameObject(GameObject gameObject) { m_GameObject = std::move(gameObject); }

	private:
		GameObject m_GameObject;
	};
}

#define GE_NATIVE_SCRIPT_PASTE(a, b) a##b
#define GE_NATIVE_SCRIPT_EVALUATE(a, b) GE_NATIVE_SCRIPT_PASTE(a, b)
#define GE_REGISTER_NATIVE_SCRIPT(Type, Name) \
	namespace { \
		const bool GE_NATIVE_SCRIPT_EVALUATE(GeNativeScriptRegistered_, __COUNTER__) = \
			GEngine::ScriptEngine::RegisterNativeScript(Name, [] { \
				return std::unique_ptr<GEngine::NativeScriptBehaviour>(new Type()); \
			}); \
	}
