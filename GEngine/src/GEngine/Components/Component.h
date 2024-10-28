#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Object/GameObject.h"
namespace GEngine
{
	// Base class for all components
	// Each component is attached to a game object
	class GENGINE_API Component
	{
	public:
		Component() = default;

		virtual void OnRender() {};
		virtual void OnAwake() {};
		virtual void OnStart() {};
		virtual void OnUpdate() {};
		virtual void OnPhysicsUpdate() {};
		virtual void OnLateUpdate() {};
		virtual void OnDestroy() {};



		void SetGameObject(GameObject gameObject) { m_GameObject = gameObject; }
		GameObject m_GameObject;

		operator bool() { return this != nullptr; }
	};
}