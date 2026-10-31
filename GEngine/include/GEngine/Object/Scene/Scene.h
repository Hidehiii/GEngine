#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Core/UUID.h"
#include "GEngine/Object/Object.h"
#include "GEngine/Object/Transform.h"
#include <entt.hpp>
#include <vector>
#include <string>

namespace GEngine
{
    class GameObject;
    class CameraComponent;

    struct GameObjectData
    {
        UUID Id;
        std::string Name = "GameObject";
        bool Active = true;
        bool Awake = false;
        bool Started = false;
        Transform Spatial;
        entt::entity Parent = entt::null;
        std::vector<std::string> Scripts;
    };

    class GENGINE_API Scene : private Object
    {
    public:
        Scene() = default;
        ~Scene();
        static Ref<Scene> Copy(Ref<Scene> scene);
        GameObject CreateGameObject(const std::string& name = {});
        GameObject CreateGameObject(uint64_t uuid, const std::string& name);
        void DuplicateGameObject(GameObject gameObject);
        void DestroyGameObject(GameObject gameObject);
        CameraComponent* MainCamera();
        void OnAwake();
        void OnStart();
        void OnFixedUpdate();
        void OnUpdate();
        void OnUpdate(float deltaTime);
        void OnLateUpdate();
        void OnEndFrame();
        void OnPause();
        void OnResume();
        void OnViewportResize(uint32_t width, uint32_t height);

        template<typename... Types>
        std::vector<GameObject> GetGameObjects()
        {
            auto view = m_Registry.view<GameObjectData, Types...>();
            std::vector<GameObject> objects;
            for (auto entity : view) objects.emplace_back(entity, this);
            return objects;
        }
        std::string name = "Untitled";
    private:
        template<typename T>
        void OnComponentAdded(GameObject& object, T& component)
        {
            if constexpr (std::is_same<T, CameraComponent>::value)
                component.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
        }
        entt::registry m_Registry;
        uint32_t m_ViewportWidth = 1920, m_ViewportHeight = 1080;
        std::vector<GameObject> m_DeletedGameObject;
        bool m_Paused = false;
        bool m_Awake = false;
        bool m_Started = false;
        float m_FixedUpdateAccumulator = 0.0f;
        void OnGameObjectActiveChanged(GameObject object);
        friend class GameObject;
        friend class Serializer;
    };
}



