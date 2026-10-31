#include "GEpch.h"
#include "GEngine/Object/GameObject.h"
#include "GEngine/Components/Components.h"
#include "GEngine/Core/Time.h"
#include "GEngine/Scripting/ScriptEngine.h"

namespace GEngine
{
    GameObject::GameObject(entt::entity handle, Scene* scene) : m_Scene(scene)
    { m_EntityHandle = handle; }
    GameObject::GameObject(uint32_t handle, Scene* scene) : GameObject(static_cast<entt::entity>(handle), scene) {}
    GameObject::operator bool() const
    { return m_Scene && m_Scene->m_Registry.valid(m_EntityHandle); }
    GameObjectData& GameObject::Data()
    {
        if (!*this) throw std::logic_error("Invalid GameObject handle.");
        return m_Scene->m_Registry.get<GameObjectData>(m_EntityHandle);
    }
    const GameObjectData& GameObject::Data() const
    {
        if (!*this) throw std::logic_error("Invalid GameObject handle.");
        return m_Scene->m_Registry.get<GameObjectData>(m_EntityHandle);
    }
    Transform& GameObject::GetTransform() { return Data().Spatial; }
    const Transform& GameObject::GetTransform() const { return Data().Spatial; }
    UUID GameObject::GetUUID() const { return Data().Id; }
    const std::string& GameObject::GetName() const { return Data().Name; }
    void GameObject::SetName(const std::string& name) { Data().Name = name; }
    bool GameObject::IsActive() const { return Data().Active; }
    void GameObject::SetActive(bool active)
    {
        auto& data = Data();
        if (data.Active == active) return;
        data.Active = active;
        m_Scene->OnGameObjectActiveChanged(*this);
    }
    GameObject GameObject::Parent() const
    { return GameObject(Data().Parent, m_Scene); }
    Scene* GameObject::GetScene() const { return m_Scene; }
    void GameObject::SetParent(GameObject parent)
    {
        Data();
        if (parent.m_EntityHandle != entt::null && (!parent || parent.m_Scene != m_Scene))
            throw std::invalid_argument("Parent must belong to the same scene.");
        for (auto ancestor = parent; ancestor; ancestor = ancestor.Parent())
            if (ancestor == *this) throw std::invalid_argument("GameObject hierarchy cannot contain a cycle.");
        Data().Parent = parent ? parent.m_EntityHandle : entt::null;
    }

    void GameObject::AddScript(const std::string& className)
    {
        auto& data = Data();
        if (className.empty()) throw std::invalid_argument("Script class name cannot be empty.");
        if (HasScript(className)) throw std::logic_error("GameObject already has this script.");
        data.Scripts.push_back(className);
    }

    void GameObject::RemoveScript(const std::string& className)
    {
        auto& data = Data();
        const auto found = std::find(data.Scripts.begin(), data.Scripts.end(), className);
        if (found == data.Scripts.end()) throw std::logic_error("GameObject does not have this script.");
        ScriptEngine::RemoveScript(*this, className);
        data.Scripts.erase(found);
    }

    bool GameObject::HasScript(const std::string& className) const
    {
        const auto& scripts = Data().Scripts;
        return std::find(scripts.begin(), scripts.end(), className) != scripts.end();
    }

    const std::vector<std::string>& GameObject::GetScriptClasses() const { return Data().Scripts; }

    void GameObject::OnAwake()
    {
        if (!*this || !IsActive()) return;
        auto& data = Data();
        if (data.Awake) return;
        if (auto* image = TryGetComponent<ImageComponent>()) image->OnAwake();
        if (auto* mesh = TryGetComponent<StaticMeshComponent>()) mesh->OnAwake();
        if (auto* camera = TryGetComponent<CameraComponent>()) camera->OnAwake();
        if (auto* light = TryGetComponent<DirectionalLightComponent>()) light->OnAwake();
        ScriptEngine::OnGameObjectAwake(*this);
        data.Awake = true;
    }

    void GameObject::OnStart()
    {
        if (!*this || !IsActive()) return;
        auto& data = Data();
        if (!data.Awake || data.Started) return;
        if (auto* image = TryGetComponent<ImageComponent>()) image->OnStart();
        if (auto* mesh = TryGetComponent<StaticMeshComponent>()) mesh->OnStart();
        if (auto* camera = TryGetComponent<CameraComponent>()) camera->OnStart();
        if (auto* light = TryGetComponent<DirectionalLightComponent>()) light->OnStart();
        ScriptEngine::OnGameObjectStart(*this);
        data.Started = true;
    }

    void GameObject::OnEnable()
    {
        if (!*this || !IsActive() || !Data().Awake) return;
        if (auto* image = TryGetComponent<ImageComponent>()) image->OnEnable();
        if (auto* mesh = TryGetComponent<StaticMeshComponent>()) mesh->OnEnable();
        if (auto* camera = TryGetComponent<CameraComponent>()) camera->OnEnable();
        if (auto* light = TryGetComponent<DirectionalLightComponent>()) light->OnEnable();
        ScriptEngine::OnGameObjectEnable(*this);
    }

    void GameObject::OnDisable()
    {
        if (!*this || !Data().Awake) return;
        if (auto* image = TryGetComponent<ImageComponent>()) image->OnDisable();
        if (auto* mesh = TryGetComponent<StaticMeshComponent>()) mesh->OnDisable();
        if (auto* camera = TryGetComponent<CameraComponent>()) camera->OnDisable();
        if (auto* light = TryGetComponent<DirectionalLightComponent>()) light->OnDisable();
        ScriptEngine::OnGameObjectDisable(*this);
    }

    void GameObject::OnFixedUpdate(float fixedDeltaTime)
    {
        if (!*this || !IsActive() || !Data().Started) return;
        if (auto* image = TryGetComponent<ImageComponent>()) image->OnFixedUpdate();
        if (auto* mesh = TryGetComponent<StaticMeshComponent>()) mesh->OnFixedUpdate();
        if (auto* camera = TryGetComponent<CameraComponent>()) camera->OnFixedUpdate();
        if (auto* light = TryGetComponent<DirectionalLightComponent>()) light->OnFixedUpdate();
        if (auto* image = TryGetComponent<ImageComponent>()) image->OnPhysicsUpdate();
        if (auto* mesh = TryGetComponent<StaticMeshComponent>()) mesh->OnPhysicsUpdate();
        if (auto* camera = TryGetComponent<CameraComponent>()) camera->OnPhysicsUpdate();
        ScriptEngine::OnGameObjectFixedUpdate(*this, fixedDeltaTime);
    }

    void GameObject::OnPhysicsUpdate(float fixedDeltaTime) { OnFixedUpdate(fixedDeltaTime); }

    void GameObject::OnUpdate() { OnUpdate(Time::GetDeltaTime()); }

    void GameObject::OnUpdate(float deltaTime)
    {
        if (!*this || !IsActive() || !Data().Started) return;
        if (auto* image = TryGetComponent<ImageComponent>()) image->OnUpdate();
        if (auto* mesh = TryGetComponent<StaticMeshComponent>()) mesh->OnUpdate();
        if (auto* camera = TryGetComponent<CameraComponent>()) camera->OnUpdate();
        if (auto* light = TryGetComponent<DirectionalLightComponent>()) light->OnUpdate();
        ScriptEngine::OnGameObjectUpdate(*this, deltaTime);
    }

    void GameObject::OnLateUpdate()
    {
        if (!*this || !IsActive() || !Data().Started) return;
        if (auto* image = TryGetComponent<ImageComponent>()) image->OnLateUpdate();
        if (auto* mesh = TryGetComponent<StaticMeshComponent>()) mesh->OnLateUpdate();
        if (auto* camera = TryGetComponent<CameraComponent>()) camera->OnLateUpdate();
        if (auto* light = TryGetComponent<DirectionalLightComponent>()) light->OnLateUpdate();
        ScriptEngine::OnGameObjectLateUpdate(*this);
    }

    void GameObject::OnPause()
    {
        if (!*this || !IsActive()) return;
        if (auto* image = TryGetComponent<ImageComponent>()) image->OnPause();
        if (auto* mesh = TryGetComponent<StaticMeshComponent>()) mesh->OnPause();
        if (auto* camera = TryGetComponent<CameraComponent>()) camera->OnPause();
        if (auto* light = TryGetComponent<DirectionalLightComponent>()) light->OnPause();
        ScriptEngine::OnGameObjectPause(*this);
    }

    void GameObject::OnResume()
    {
        if (!*this || !IsActive()) return;
        if (auto* image = TryGetComponent<ImageComponent>()) image->OnResume();
        if (auto* mesh = TryGetComponent<StaticMeshComponent>()) mesh->OnResume();
        if (auto* camera = TryGetComponent<CameraComponent>()) camera->OnResume();
        if (auto* light = TryGetComponent<DirectionalLightComponent>()) light->OnResume();
        ScriptEngine::OnGameObjectResume(*this);
    }

    void GameObject::OnDestroy()
    {
        if (!*this) return;
        if (auto* image = TryGetComponent<ImageComponent>()) image->OnDestroy();
        if (auto* mesh = TryGetComponent<StaticMeshComponent>()) mesh->OnDestroy();
        if (auto* camera = TryGetComponent<CameraComponent>()) camera->OnDestroy();
        if (auto* light = TryGetComponent<DirectionalLightComponent>()) light->OnDestroy();
        ScriptEngine::OnGameObjectDestroy(*this);
    }

    void GameObject::OnRender()
    {
        if (!IsActive()) return;
        if (auto* image = TryGetComponent<ImageComponent>()) image->OnRender();
        if (auto* mesh = TryGetComponent<StaticMeshComponent>()) mesh->OnRender();
    }
}


