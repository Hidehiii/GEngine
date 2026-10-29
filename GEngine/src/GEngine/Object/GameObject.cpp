#include "GEpch.h"
#include "GEngine/Object/GameObject.h"
#include "GEngine/Components/Components.h"

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
    void GameObject::SetActive(bool active) { Data().Active = active; }
    GameObject GameObject::Parent() const
    { return GameObject(Data().Parent, m_Scene); }
    void GameObject::SetParent(GameObject parent)
    {
        Data();
        if (parent.m_EntityHandle != entt::null && (!parent || parent.m_Scene != m_Scene))
            throw std::invalid_argument("Parent must belong to the same scene.");
        for (auto ancestor = parent; ancestor; ancestor = ancestor.Parent())
            if (ancestor == *this) throw std::invalid_argument("GameObject hierarchy cannot contain a cycle.");
        Data().Parent = parent ? parent.m_EntityHandle : entt::null;
    }
    void GameObject::OnUpdate() {}
    void GameObject::OnRender()
    {
        if (!IsActive()) return;
        if (auto* image = TryGetComponent<ImageComponent>()) image->OnRender();
        if (auto* mesh = TryGetComponent<StaticMeshComponent>()) mesh->OnRender();
    }
}


