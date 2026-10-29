#pragma once
#include "GEngine/Object/Scene/Scene.h"
#include <stdexcept>
#include <type_traits>

namespace GEngine
{
    class StaticMeshComponent;
    class ImageComponent;
    class CameraComponent;

    template<typename T>
    constexpr bool IsSceneComponent = std::is_same_v<T, StaticMeshComponent> ||
        std::is_same_v<T, ImageComponent> || std::is_same_v<T, CameraComponent>;

    class GENGINE_API GameObject : public Object
    {
    public:
        GameObject() = default;
        GameObject(entt::entity handle, Scene* scene);
        GameObject(uint32_t handle, Scene* scene);
        ~GameObject() = default;
        GameObject(const GameObject&) = default;

        Transform& GetTransform();
        const Transform& GetTransform() const;
        UUID GetUUID() const;
        const std::string& GetName() const;
        void SetName(const std::string& name);
        bool IsActive() const;
        void SetActive(bool active);
        GameObject Parent() const;
        void SetParent(GameObject parent = {});
        void OnUpdate();
        void OnRender();
        operator bool() const;

        template<typename T> bool HasComponent() const
        {
            static_assert(IsSceneComponent<T>, "Only StaticMeshComponent, ImageComponent and CameraComponent are scene components.");
            return *this && m_Scene->m_Registry.any_of<T>(m_EntityHandle);
        }
        template<typename T, typename... Args> void AddComponent(Args&&... args)
        {
            static_assert(IsSceneComponent<T>, "Only StaticMeshComponent, ImageComponent and CameraComponent are scene components.");
            Data();
            if (HasComponent<T>()) throw std::logic_error("GameObject already has this component.");
            auto& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
            component.AttachGameObject(*this);
            m_Scene->OnComponentAdded(*this, component);
        }
        template<typename T, typename... Args> void AddOrReplaceComponent(Args&&... args)
        {
            static_assert(IsSceneComponent<T>, "Only StaticMeshComponent, ImageComponent and CameraComponent are scene components.");
            Data();
            auto& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
            component.AttachGameObject(*this);
            m_Scene->OnComponentAdded(*this, component);
        }
        template<typename T> T& GetComponent()
        {
            auto* component = TryGetComponent<T>();
            if (!component) throw std::logic_error("GameObject does not have this component.");
            return *component;
        }
        template<typename T> T* TryGetComponent()
        {
            static_assert(IsSceneComponent<T>, "Only StaticMeshComponent, ImageComponent and CameraComponent are scene components.");
            return *this ? m_Scene->m_Registry.try_get<T>(m_EntityHandle) : nullptr;
        }
        template<typename T> void RemoveComponent()
        {
            static_assert(IsSceneComponent<T>, "Only StaticMeshComponent, ImageComponent and CameraComponent are scene components.");
            Data();
            m_Scene->m_Registry.remove<T>(m_EntityHandle);
        }
        bool operator==(const GameObject& other) const
        { return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; }
        bool operator!=(const GameObject& other) const { return !(*this == other); }
    private:
        GameObjectData& Data();
        const GameObjectData& Data() const;
        Scene* m_Scene = nullptr;
        friend class Scene;
    };
}


