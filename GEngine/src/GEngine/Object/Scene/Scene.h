#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Object/Object.h"
#include <entt.hpp>

class b2World;

namespace GEngine
{
	class GameObject;
	class Camera;
	class DirectionalLight;


	class GENGINE_API Scene : private Object
	{
	public:
		Scene();
		~Scene();

		static Ref<Scene> Copy(Ref<Scene> scene);

		GameObject CreateGameObject(const std::string& name = std::string());
		GameObject CreateGameObject(uint32_t uuid, const std::string& name);
		void DuplicateGameObject(GameObject gameObject);
		void DestroyGameObject(GameObject gameObject);
		Camera& MainCamera();
		DirectionalLight& MainDirectionalLight();
		// Logic and physics 
		void OnAwake();
		void OnStart();
		void OnUpdate();
		void OnEndFrame();

		// Render
		void OnRender();

		void OnViewportResize(uint32_t width, uint32_t height);
	public:
		std::string name = "Untitled";
	private:
		template<typename T>
		void OnComponentAdded(GameObject gameObject, T& component);
		/*template<typename T>
		void OnComponentRemoved(GameObject gameObject, T& component);

		template<typename T>
		void OnComponentAdded(ScriptableObject scriptableObject, T& component);
		template<typename T>
		void OnComponentRemoved(ScriptableObject scriptableObject, T& component);*/
	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		std::vector<GameObject> m_DeletedGameObject;

		b2World* m_PhysicsWorld2D = nullptr;

		friend class GameObject;
		friend class Serializer;
		friend class Hierarchy;
	};
}



