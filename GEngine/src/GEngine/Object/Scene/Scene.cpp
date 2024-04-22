#include "GEpch.h"
#include "Scene.h"
#include "GEngine/Core/Time.h"
#include "GEngine/Renderer/Renderer2D.h"
#include "GEngine/Editor/Renderer/EditorRenderer2D.h"
#include "GEngine/Components/Components.h"
#include "GEngine/Object/GameObject.h"


// Box2D
#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>

namespace GEngine
{

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap, const Ref<Scene> scene)
	{
		([&]()
			{
				auto view = src.view<Component>();
				for (auto srcEntity : view)
				{
					GE_CORE_ASSERT(enttMap.find(src.get<ID>(srcEntity).GetUUID()) != enttMap.end(), "Entity not found in map!");
					entt::entity dstEntity = enttMap.at(src.get<ID>(srcEntity).GetUUID());

					auto& srcComponent = src.get<Component>(srcEntity);
					Component& component = dst.emplace_or_replace<Component>(dstEntity, srcComponent);
					component.SetGameObject({ dstEntity, scene.get() });
				}
			}(), ...);
	};

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap, const Ref<Scene> scene)
	{
		CopyComponent<Component...>(dst, src, enttMap, scene);
	};

	template<typename... Component>
	static void CopyComponentIfExists(GameObject dst, GameObject src)
	{
		([&]()
			{
				if (src.HasComponent<Component>())
					dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponentIfExists(ComponentGroup<Component...>, GameObject dst, GameObject src)
	{
		CopyComponentIfExists<Component...>(dst, src);
	}

	class GameObject;
	class Camera;

	static b2BodyType RigidBodyTypeToBox2D(RigidBody2D::RigidBodyType type)
	{
		switch (type)
		{
			case RigidBody2D::RigidBodyType::Static: return b2_staticBody;
			case RigidBody2D::RigidBodyType::Dynamic: return b2_dynamicBody;
			case RigidBody2D::RigidBodyType::Kinematic: return b2_kinematicBody;
		}
		GE_CORE_ASSERT(false, "Unknown RigidBodyType!");
		return b2_staticBody;
	}

	Scene::Scene()
	{
		
	}
	Scene::~Scene()
	{
	}
	Ref<Scene> Scene::Copy(Ref<Scene> scene)
	{
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->name = scene->name;

		auto& srcSceneRegistry = scene->m_Registry;
		auto& dstSceneRegistry = newScene->m_Registry;

		std::unordered_map<UUID, entt::entity> enttMap;

		auto view = srcSceneRegistry.view<ID>();
		for (auto entity : view)
		{
			UUID id = srcSceneRegistry.get<ID>(entity).GetUUID();
			auto& name = srcSceneRegistry.get<Name>(entity).m_Name;
			auto newEntity = newScene->CreateGameObject(id, name);
			enttMap[id] = (entt::entity)newEntity;
		}

		CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap, newScene);

		return newScene;
	}
	GameObject Scene::CreateGameObject(const std::string& name)
	{
		GameObject object = CreateGameObject(UUID(), name);
		return object;
	}
	GameObject Scene::CreateGameObject(uint32_t uuid, const std::string& name)
	{
		GameObject object = { m_Registry.create(), this };
		object.AddComponent<Attribute>();
		object.AddComponent<ID>(uuid);
		object.AddComponent<Transform>();
		auto& n = object.AddComponent<Name>();
		n.m_Name = name.empty() ? "GameObject" : name;
		return object;
	}
	void Scene::DuplicateGameObject(GameObject gameObject)
	{
		GameObject newObject = CreateGameObject(gameObject.GetComponent<Name>().m_Name);

		CopyComponentIfExists(AllComponents{}, newObject, gameObject);
	}
	void Scene::DestroyGameObject(GameObject gameObject)
	{
		m_DeletedGameObject.push_back(gameObject);
	}
	Camera& Scene::MainCamera()
	{
		auto view = m_Registry.view<Camera>();
		if (view.size() == 0)
		{
			return Camera();
		}
		for (auto entity : view)
		{
			auto& camera = m_Registry.get<Camera>(entity);
			if (camera.isPrimary)
			{
				return camera;
			}
		}
		for (auto entity : view)
		{
			auto& camera = m_Registry.get<Camera>(entity);
			camera.isPrimary = true;
			return camera;
		}
	}
	void Scene::OnAwake()
	{
		// Awake Scripts
		m_Registry.view<NativeScript>().each([=](auto entity, auto& script)
			{
				if (!script.Instance)
				{
					script.Instance = script.InstantiateFunc();
					script.Instance->m_GameObject = GameObject{ entity, this };
					script.Instance->OnAwake();
				}
			});
		// Add physics world 2D
		m_PhysicsWorld2D = new b2World(b2Vec2(0.0f, -9.8f));
		auto view = m_Registry.view<RigidBody2D>();
		for (auto rig : view)
		{
			GameObject gameObject = m_Registry.get<RigidBody2D>(rig).m_GameObject;
			auto& transform = gameObject.GetComponent<Transform>();
			auto& rigidBody = gameObject.GetComponent<RigidBody2D>();

			b2BodyDef bodyDef;
			bodyDef.type = RigidBodyTypeToBox2D(rigidBody.m_RigidBodyType);
			bodyDef.position.Set(transform.m_Position.value.x, transform.m_Position.value.y);
			bodyDef.angle = transform.GetEulerAngles().value.z;

			b2Body* body =  m_PhysicsWorld2D->CreateBody(&bodyDef);
			body->SetFixedRotation(rigidBody.m_FixedRotation);
			rigidBody.m_Body = body;

			if (gameObject.HasComponent<BoxCollider2D>())
			{
				auto& boxCollider = gameObject.GetComponent<BoxCollider2D>();

				b2PolygonShape shape; 
				shape.SetAsBox(boxCollider.m_Size.value.x / 2.0f * transform.m_Scale.value.x, boxCollider.m_Size.value.y / 2.0f * transform.m_Scale.value.y, 
					b2Vec2(boxCollider.m_Offset.value.x, boxCollider.m_Offset.value.y), Math::Radians(boxCollider.m_Rotation));

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &shape;
				fixtureDef.density = boxCollider.m_Density;
				fixtureDef.friction = boxCollider.m_Friction;
				fixtureDef.restitution = boxCollider.m_Restitution;
				fixtureDef.restitutionThreshold = boxCollider.m_RestitutionThreshold;
				body->CreateFixture(&fixtureDef);
			}
			if (gameObject.HasComponent<CircleCollider2D>())
			{
				auto& circleCollider = gameObject.GetComponent<CircleCollider2D>();

				b2CircleShape shape;
				shape.m_p.Set(circleCollider.m_Offset.value.x, circleCollider.m_Offset.value.y);
				shape.m_radius = Math::Max(transform.m_Scale) * circleCollider.m_Radius;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &shape;
				fixtureDef.density = circleCollider.m_Density;
				fixtureDef.friction = circleCollider.m_Friction;
				fixtureDef.restitution = circleCollider.m_Restitution;
				fixtureDef.restitutionThreshold = circleCollider.m_RestitutionThreshold;
				body->CreateFixture(&fixtureDef);
			}
		}
	}
	void Scene::OnStart()
	{
		// Start Scripts
		m_Registry.view<NativeScript>().each([=](auto entity, auto& script)
			{
				if (!script.Instance)
				{
					script.Instance = script.InstantiateFunc();
					script.Instance->m_GameObject = GameObject{ entity, this };
					script.Instance->OnAwake();
				}
				script.Instance->OnStart();
			});
	}
	void Scene::OnUpdate()
	{

		// Update Scripts
		{
			m_Registry.view<NativeScript>().each([=](auto entity, auto& script)
				{
					if (!script.Instance)
					{
						script.Instance = script.InstantiateFunc();
						script.Instance->m_GameObject = GameObject{ entity, this };
						script.Instance->OnAwake();
						script.Instance->OnStart();
					}
					script.Instance->OnUpdate();
				});
		}

		// Physics 2D
		{
			const uint32_t velocityIterations = 6;
			const uint32_t positionIterations = 2;
			m_PhysicsWorld2D->Step(Time::GetDeltaTime(), velocityIterations, positionIterations);

			// retrieve transform
			auto view = m_Registry.view<RigidBody2D>();
			for (auto entity : view)
			{
				GameObject gameObject = m_Registry.get<RigidBody2D>(entity).m_GameObject;
				auto& transform = gameObject.GetComponent<Transform>();
				auto& rigidBody = gameObject.GetComponent<RigidBody2D>();

				b2Body* body = (b2Body*)rigidBody.m_Body;
				const auto& pos = body->GetPosition();
				transform.m_Position.value.x = pos.x;
				transform.m_Position.value.y = pos.y;
				transform.SetEulerAngleInRadians({ 0.0f, 0.0f, body->GetAngle() });
			}
		}
	}
	void Scene::OnEndFrame()
	{
		for(auto object : m_DeletedGameObject)
		{
			m_Registry.destroy(object);
		}
		m_DeletedGameObject.clear();
	}
	void Scene::OnRender()
	{
		auto view_ImagerRenderer = m_Registry.view<Transform, ImageRenderer>();
		for (auto entity : view_ImagerRenderer)
		{
			GameObject& gameObject = m_Registry.get<ImageRenderer>(entity).m_GameObject;
			if (gameObject.GetComponent<Attribute>().m_IsActive)
			{
				Renderer2D::RenderImage(gameObject.GetComponent<Transform>(), gameObject.GetComponent<ImageRenderer>());
			}
		}
		auto view_CircleRenderer = m_Registry.view<Transform, CircleRenderer>();
		for (auto entity : view_CircleRenderer)
		{
			GameObject& gameObject = m_Registry.get<CircleRenderer>(entity).m_GameObject;
			if (gameObject.GetComponent<Attribute>().m_IsActive)
			{
				Renderer2D::RenderCircle(gameObject.GetComponent<Transform>(), gameObject.GetComponent<CircleRenderer>());
			}
		}
		auto view_MeshRenderer = m_Registry.view<Transform, MeshFilter, MeshRenderer>();
		for(auto entity : view_MeshRenderer)
		{
			GameObject& gameObject = m_Registry.get<MeshRenderer>(entity).m_GameObject;
			if (gameObject.GetComponent<Attribute>().m_IsActive)
			{
				gameObject.GetComponent<MeshRenderer>().OnRender();
			}
		}
	}
	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		auto view = m_Registry.view<Camera>();
		for (auto entity : view)
		{
			auto& camera = view.get<Camera>(entity);
			camera.SetViewportSize(width, height);
		}
	}


	template <typename T>
	void Scene::OnComponentAdded(GameObject gameObject, T& component)
	{
		//static_assert(false);
	}
	template <>
	void GENGINE_API Scene::OnComponentAdded<Name>(GameObject gameObject, Name& component)
	{
	}
	template <>
	void GENGINE_API Scene::OnComponentAdded<Transform>(GameObject gameObject, Transform& component)
	{
	}
	template <>
	void GENGINE_API Scene::OnComponentAdded<Camera>(GameObject gameObject, Camera& component)
	{
		component.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}
	template <>
	void GENGINE_API Scene::OnComponentAdded<ImageRenderer>(GameObject gameObject, ImageRenderer& component)
	{
	}
	template <>
	void GENGINE_API Scene::OnComponentAdded<RigidBody2D>(GameObject gameObject, RigidBody2D& component)
	{
	}
	template <>
	void GENGINE_API Scene::OnComponentAdded<BoxCollider2D>(GameObject gameObject, BoxCollider2D& component)
	{
	}
	template <>
	void GENGINE_API Scene::OnComponentAdded<CircleCollider2D>(GameObject gameObject, CircleCollider2D& component)
	{
	}
	template <>
	void GENGINE_API Scene::OnComponentAdded<NativeScript>(GameObject gameObject, NativeScript& component)
	{
	}
	template <>
	void GENGINE_API Scene::OnComponentAdded<CircleRenderer>(GameObject gameObject, CircleRenderer& component)
	{
	}
	template <>
	void GENGINE_API Scene::OnComponentAdded<MeshFilter>(GameObject gameObject, MeshFilter& component)
	{
	}
	template <>
	void GENGINE_API Scene::OnComponentAdded<MeshRenderer>(GameObject gameObject, MeshRenderer& component)
	{
		component.Init();
	}
}