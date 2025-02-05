#include "GEpch.h"
#include "GameObject.h"
#include "GEngine/Object/Scene/Scene.h"
#include "GEngine/Components/Components.h"

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

	GameObject::GameObject(uint32_t handle, Scene* scene)
		: m_Scene(scene)
	{
		m_EntityHandle = (entt::entity)handle;
	}




	GameObject::~GameObject()
	{
	}

	GameObject* GameObject::Parent()
	{
		return &(GetComponent<Transform>().Parent()->m_GameObject);
	}

	void GameObject::SetParent(GameObject* parent)
	{
		GetComponent<Transform>().SetParent(parent->GetComponent<Transform>());
	}

	void GameObject::OnUpdate()
	{
	}

	void GameObject::OnRender()
	{
	}
}


