#include "GEpch.h"
#include "GEngine/Object/Scene/Scene.h"
#include "GEngine/Object/GameObject.h"
#include "GEngine/Components/Components.h"
#include "GEngine/Core/Time.h"
#include <stdexcept>

namespace GEngine
{
    template<typename... T>
    static void CopyComponents(ComponentGroup<T...>, GameObject destination, GameObject source)
    {
        ([&] { if (source.HasComponent<T>()) destination.AddOrReplaceComponent<T>(source.GetComponent<T>()); }(), ...);
    }
    template<typename Function>
    static void ForComponents(Scene& scene, Function function)
    {
        for (auto object : scene.GetGameObjects<>())
        {
            if (!object.IsActive()) continue;
            if (auto* component = object.TryGetComponent<StaticMeshComponent>()) function(*component);
            if (auto* component = object.TryGetComponent<ImageComponent>()) function(*component);
            if (auto* component = object.TryGetComponent<CameraComponent>()) function(*component);
        }
    }
    Scene::~Scene()
    {
        for (auto object : GetGameObjects<>()) object.OnDestroy();
    }
    Ref<Scene> Scene::Copy(Ref<Scene> scene)
    {
        if (!scene) throw std::invalid_argument("Cannot copy a null scene.");
        auto result = CreateRef<Scene>();
        result->name = scene->name;
        result->m_ViewportWidth = scene->m_ViewportWidth;
        result->m_ViewportHeight = scene->m_ViewportHeight;
        std::unordered_map<uint32_t, GameObject> mapping;
        for (auto source : scene->GetGameObjects<>())
        {
            auto destination = result->CreateGameObject(source.GetUUID(), source.GetName());
            destination.GetTransform() = source.GetTransform();
            destination.SetActive(source.IsActive());
            for (const auto& script : source.GetScriptClasses()) destination.AddScript(script);
            CopyComponents(AllComponents{}, destination, source);
            mapping.emplace(static_cast<uint32_t>(source), destination);
        }
        for (auto source : scene->GetGameObjects<>())
            if (auto parent = source.Parent())
                mapping.at(static_cast<uint32_t>(source)).SetParent(mapping.at(static_cast<uint32_t>(parent)));
        return result;
    }
    GameObject Scene::CreateGameObject(const std::string& name) { return CreateGameObject(UUID(), name); }
    GameObject Scene::CreateGameObject(uint64_t uuid, const std::string& name)
    {
        for (auto existing : GetGameObjects<>())
            if (static_cast<uint64_t>(existing.GetUUID()) == uuid) throw std::invalid_argument("Duplicate GameObject UUID.");
        const auto entity = m_Registry.create();
        auto& data = m_Registry.emplace<GameObjectData>(entity);
        data.Id = uuid;
        data.Name = name.empty() ? "GameObject" : name;
        return GameObject(entity, this);
    }
    void Scene::DuplicateGameObject(GameObject source)
    {
        if (!source || source.m_Scene != this) throw std::invalid_argument("Object must belong to this scene.");
        auto destination = CreateGameObject(source.GetName());
        destination.GetTransform() = source.GetTransform();
        destination.SetActive(source.IsActive());
        destination.SetParent(source.Parent());
        for (const auto& script : source.GetScriptClasses()) destination.AddScript(script);
        CopyComponents(AllComponents{}, destination, source);
    }
    void Scene::DestroyGameObject(GameObject object)
    {
        if (!object || object.m_Scene != this) throw std::invalid_argument("Object must belong to this scene.");
        m_DeletedGameObject.push_back(object);
    }
    CameraComponent* Scene::MainCamera()
    {
        CameraComponent* first = nullptr;
        for (auto object : GetGameObjects<CameraComponent>())
        {
            auto& camera = object.GetComponent<CameraComponent>();
            if (camera.isPrimary) return &camera;
            if (!first) first = &camera;
        }
        if (first) first->isPrimary = true;
        return first;
    }
    void Scene::OnAwake()
    {
        if (m_Awake) return;
        for (auto object : GetGameObjects<>())
            if (object.IsActive())
            {
                object.OnAwake();
                object.OnEnable();
            }
        m_Awake = true;
    }

    void Scene::OnStart()
    {
        OnAwake();
        if (m_Started) return;
        for (auto object : GetGameObjects<>()) object.OnStart();
        m_Started = true;
    }

    void Scene::OnFixedUpdate()
    {
        if (m_Paused || !m_Started) return;
        for (auto object : GetGameObjects<>()) object.OnFixedUpdate(Time::GetFixedTime());
    }

    void Scene::OnUpdate() { OnUpdate(Time::GetDeltaTime()); }

    void Scene::OnUpdate(float deltaTime)
    {
        if (m_Paused) return;
        OnStart();

        m_FixedUpdateAccumulator += deltaTime;
        constexpr int maximumFixedSteps = 5;
        int fixedSteps = 0;
        while (m_FixedUpdateAccumulator >= Time::GetFixedTime() && fixedSteps < maximumFixedSteps)
        {
            OnFixedUpdate();
            m_FixedUpdateAccumulator -= Time::GetFixedTime();
            ++fixedSteps;
        }
        if (m_FixedUpdateAccumulator >= Time::GetFixedTime())
            m_FixedUpdateAccumulator = Time::GetFixedTime();

        for (auto object : GetGameObjects<>()) object.OnUpdate(deltaTime);
    }

    void Scene::OnLateUpdate()
    { if (!m_Paused) for (auto object : GetGameObjects<>()) object.OnLateUpdate(); }

    void Scene::OnEndFrame()
    {
        for (auto object : m_DeletedGameObject)
        {
            if (!object) continue;
            object.OnDestroy();
            for (auto child : GetGameObjects<>()) if (child.Parent() == object) child.SetParent();
            m_Registry.destroy(object);
        }
        m_DeletedGameObject.clear();
    }
    void Scene::OnPause()
    {
        if (m_Paused) return;
        m_Paused = true;
        for (auto object : GetGameObjects<>()) object.OnPause();
    }

    void Scene::OnResume()
    {
        if (!m_Paused) return;
        m_Paused = false;
        for (auto object : GetGameObjects<>()) object.OnResume();
    }
    void Scene::OnViewportResize(uint32_t width, uint32_t height)
    {
        if (!width || !height) return;
        m_ViewportWidth = width; m_ViewportHeight = height;
        for (auto object : GetGameObjects<CameraComponent>()) object.GetComponent<CameraComponent>().SetViewportSize(width, height);
    }

    void Scene::OnGameObjectActiveChanged(GameObject object)
    {
        if (!object || object.m_Scene != this) return;
        if (object.IsActive())
        {
            object.OnAwake();
            object.OnEnable();
            if (m_Started) object.OnStart();
        }
        else object.OnDisable();
    }
}
