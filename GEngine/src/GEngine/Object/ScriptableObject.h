#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Object/GameObject.h"
#include "GEngine/Object/Scene/Scene.h"

// temp 
#include "GEngine/Physics/2D/PhysicalContactListener2D.h"

namespace GEngine
{
	class GENGINE_API ScriptableObject : public Object
	{
	public:
		virtual ~ScriptableObject() {};

		template<typename T>
		T& AddComponent()
		{
			GE_CORE_ASSERT(!m_GameObject.HasComponent<T>(), "GameObject already has component!");
			return m_GameObject.AddComponent<T>();
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
	public:
		GameObject m_GameObject;
	};
}


