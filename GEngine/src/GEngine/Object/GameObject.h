#pragma once
#include "GEngine/Object/Scene/Scene.h"

// temp 
//#include "GEngine/Physics/2D/PhysicalContactListener2D.h"
namespace GEngine
{
	enum class ComponentFunction
	{
		OnAwake,
		OnStart,
		OnUpdate,
		OnPhysicsUpdate,
		OnLateUpdate,
		OnDestroy,
		OnRender
	};


	class ScriptableObject;
	class Physics2DContactInfo;
	

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
			if constexpr (std::is_base_of<ScriptableObject, T>::value)
			{
				return m_Scene->m_Registry.get<NativeScript>(m_EntityHandle).HasInstance<T>();
			}
			else
			{
				return m_Scene->m_Registry.any_of<T>(m_EntityHandle);
			}
		}
		template<typename T, typename... Args>
		void AddComponent(Args&&... args)
		{
			if constexpr (std::is_base_of<ScriptableObject, T>::value)
			{
				m_Scene->m_Registry.get<NativeScript>(m_EntityHandle).AddInstance<T>();
			}
			else
			{
				T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
				component.SetGameObject((*this));
				m_Scene->OnComponentAdded<T>(*this, component);
			}
		}
		template<typename T, typename... Args>
		void AddOrReplaceComponent(Args&&... args)
		{
			T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
			component.SetGameObject((*this));
			m_Scene->OnComponentAdded<T>(*this, component);
		}
		template<typename T>
		T& GetComponent()
		{
			if constexpr (std::is_base_of<ScriptableObject, T>::value)
			{
				return *(m_Scene->m_Registry.get<NativeScript>(m_EntityHandle).GetInstance<T>());
			}
			else
			{
				return m_Scene->m_Registry.get<T>(m_EntityHandle);
			}
		}
		template<typename T>
		T* TryGetComponent()
		{
			if constexpr (std::is_base_of<ScriptableObject, T>::value)
			{
				return m_Scene->m_Registry.get<NativeScript>(m_EntityHandle).GetInstance<T>();
			}
			else
			{
				return m_Scene->m_Registry.try_get<T>(m_EntityHandle);
			}
		}
		template<typename T>
		void RemoveComponent()
		{
			if constexpr (std::is_base_of<ScriptableObject, T>::value)
			{
				m_Scene->m_Registry.get<NativeScript>(m_EntityHandle).RemoveInstance<T>();
			}
			else
			{
				m_Scene->m_Registry.remove<T>(m_EntityHandle);
			}
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


	class GENGINE_API ScriptableObject : public Object
	{
	public:
		ScriptableObject() = default;
		virtual ~ScriptableObject() {};

		template<typename T, typename... Args>
		void AddComponent(Args&&... args)
		{
			GE_CORE_ASSERT(!m_GameObject.HasComponent<T>(), "GameObject already has component!");
			m_GameObject.AddComponent<T>(std::forward<Args>(args)...);
		}
		template<typename T>
		T& GetComponent()
		{
			GE_CORE_ASSERT(m_GameObject.HasComponent<T>(), "GameObject does not have component!");
			return m_GameObject.GetComponent<T>();
		}
		template<typename T>
		void RemoveComponent()
		{
			GE_CORE_ASSERT(m_GameObject.HasComponent<T>(), "GameObject does not have component!");
			m_GameObject.RemoveComponent<T>();
		}
		template<typename T>
		T* TryGetComponent()
		{
			return m_GameObject.TryGetComponent<T>();
		}
	public:
		// time logic
		virtual void OnAwake() {};
		virtual void OnStart() {};
		virtual void OnUpdate() {};
		virtual void OnPhysicsUpdate() {};
		virtual void OnLateUpdate() {};
		virtual void OnDestroy() {};

		// physics
		virtual void OnCollisionEnter2D(Ref<Physics2DContactInfo> info) {};
		virtual void OnCollisionStay2D(Ref<Physics2DContactInfo> info) {};
		virtual void OnCollisionExit2D(Ref<Physics2DContactInfo> info) {};

		// graphsics
		virtual void OnRender() {};
	public:
		GameObject m_GameObject;
	};
}


