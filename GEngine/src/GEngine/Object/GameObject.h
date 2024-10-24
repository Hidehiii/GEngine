#pragma once
#include "GEngine/Object/Scene/Scene.h"
namespace GEngine
{
	class GENGINE_API GameObject :	public Object
	{
	public:
		GameObject();
		GameObject(entt::entity handle, Scene* scene);
		GameObject(uint32_t handle, Scene* scene);
		~GameObject();
		GameObject(const GameObject&) = default;

		void OnUpdate();
		void OnRender();

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.any_of<T>(m_EntityHandle);
		}
		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			if constexpr (std::is_base_of<ScriptableObject, T>::value)
			{
				GE_TRACE("add script!");
			}
			else
			{
				GE_TRACE("no add script");
				T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
				component.SetGameObject((*this));
				m_Scene->OnComponentAdded<T>(*this, component);
				return component;
			}
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			return component;
		}
		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args)
		{
			T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
			component.SetGameObject((*this));
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}
		template<typename T>
		T& GetComponent()
		{
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}
		template<typename T>
		T* TryGetComponent()
		{
			return m_Scene->m_Registry.try_get<T>(m_EntityHandle);
		}
		template<typename T>
		void RemoveComponent()
		{
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		bool operator==(const GameObject& other) const
		{
			return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
		}
		bool operator!=(const GameObject& other) const
		{
			return !(*this == other);
		}
	private:
		Scene* m_Scene = nullptr;
	};
}


