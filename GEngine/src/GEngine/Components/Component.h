#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Object/GameObject.h"
namespace GEngine
{
	class GENGINE_API Component
	{
	public:
		Component() = default;
		void SetGameObject(GameObject gameObject) { m_GameObject = gameObject; }
		GameObject m_GameObject;
	};
}