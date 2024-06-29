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
		void SetGameObject(GameObject gameObject) { m_GameObject = gameObject; }
		GameObject m_GameObject;

		operator bool() { return this != nullptr; }
	};
}