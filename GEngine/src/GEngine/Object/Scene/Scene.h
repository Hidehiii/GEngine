#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Object/Object.h"
#include "GEngine/Core/Time.h"
#include <entt.hpp>

namespace GEngine
{
	class GameObject;
	class Camera;
	class DirectionalLight;
	class Physics2DWorld;
	class PhysicalContactListener2D;
	class Physics3DWorld;


	class GENGINE_API Scene : private Object
	{
	public:
		Scene();
		~Scene();

		static Ref<Scene> Copy(Ref<Scene> scene);

		GameObject			CreateGameObject(const std::string& name = std::string());
		GameObject			CreateGameObject(uint32_t uuid, const std::string& name);
		void				DuplicateGameObject(GameObject gameObject);
		void				DestroyGameObject(GameObject gameObject);
		Camera*				MainCamera();
		DirectionalLight*	MainDirectionalLight();
		// Logic and physics 
		void OnAwake();
		void OnStart();
		void OnUpdate();
		void OnLateUpdate();
		void OnEndFrame();

		void OnPause();
		void OnResume();

		void OnViewportResize(uint32_t width, uint32_t height);

	public:
		template<typename... types>
		std::vector<GameObject> GetGameObjects()
		{
			auto view = m_Registry.view<types...>();
			std::vector<GameObject> entities;
			for (auto it : view)
			{
				entities.push_back({ *it, this });
			}
			return entities;
		}
	public:
		std::string name = "Untitled";
	private:
		template <typename T>
		void OnComponentAdded(GameObject gameObject, T& component)
		{
			if constexpr (std::is_same<T, Camera>::value)
				component.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
		}
		/*template<typename T>
		void OnComponentRemoved(GameObject gameObject, T& component);

		template<typename T>
		void OnComponentAdded(ScriptableObject scriptableObject, T& component);
		template<typename T>
		void OnComponentRemoved(ScriptableObject scriptableObject, T& component);*/
	private:
		entt::registry					m_Registry;
		uint32_t						m_ViewportWidth = 1920, m_ViewportHeight = 1080;
		std::vector<GameObject>			m_DeletedGameObject;

		Ref<PhysicsTimerWheel>			m_PhysicsTimerWheel = nullptr;

		Ref<Physics2DWorld>				m_PhysicsWorld2D = nullptr;
		Ref<PhysicalContactListener2D>	m_PhysicalContactListener2D = nullptr;

		Ref<Physics3DWorld>				m_PhysicsWorld3D = nullptr;

		friend class GameObject;
		friend class Serializer;
		friend class Hierarchy;
	};
}



