#include "GEpch.h"
#include "GameObject.h"
#include "GEngine/Object/Scene/Scene.h"

namespace GEngine
{
	GameObject::GameObject()
		: m_Scene(nullptr)
	{
	}

	GameObject::GameObject(entt::entity handle, Scene* scene)
		: m_Scene(scene)
	{
		m_EntityHandle = handle;
	}




	GameObject::~GameObject()
	{
	}

	void GameObject::OnUpdate()
	{
	}

	void GameObject::OnRender()
	{
	}
}


