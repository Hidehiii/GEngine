#include "GEpch.h"
#include "Scene.h"
#include "GEngine/Core/Time.h"
#include "GEngine/Renderer/Renderer.h"
#include "GEngine/Renderer/Renderer2D.h"
#include "GEngine/Editor/Renderer/EditorRenderer2D.h"
#include "GEngine/Components/Components.h"
#include "GEngine/Object/GameObject.h"
#include "GEngine/Physics/2D/Physics2D.h"
#include "GEngine/Physics/2D/PhysicalContactListener2D.h"


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
					GE_CORE_ASSERT(enttMap.find(src.get<Attribute>(srcEntity).GetUUID()) != enttMap.end(), "Entity not found in map!");
					entt::entity dstEntity = enttMap.at(src.get<Attribute>(srcEntity).GetUUID());

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

	Scene::Scene()
	{
		
	}
	Scene::~Scene()
	{
		if (m_PhysicsTimerWheel != nullptr)
		{
			m_PhysicsTimerWheel->Stop();
		}
	}
	Ref<Scene> Scene::Copy(Ref<Scene> scene)
	{
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->name = scene->name;

		auto& srcSceneRegistry = scene->m_Registry;
		auto& dstSceneRegistry = newScene->m_Registry;

		std::unordered_map<UUID, entt::entity> enttMap;

		auto view = srcSceneRegistry.view<Attribute>();
		for (auto entity : view)
		{
			auto& attribute = srcSceneRegistry.get<Attribute>(entity);
			UUID id = attribute.GetUUID();
			auto& name = attribute.m_Name;
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
		object.GetComponent<Attribute>().m_UUID = uuid;
		object.GetComponent<Attribute>().m_Name = name.empty() ? "GameObject" : name;
		object.AddComponent<Transform>();
		return object;
	}
	void Scene::DuplicateGameObject(GameObject gameObject)
	{
		GameObject newObject = CreateGameObject(gameObject.GetComponent<Attribute>().m_Name);

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
	DirectionalLight& Scene::MainDirectionalLight()
	{
		auto view = m_Registry.view<DirectionalLight>();
		if (view.size() == 0)
		{
			return DirectionalLight();
		}
		for (auto entity : view)
		{
			auto& light = m_Registry.get<DirectionalLight>(entity);
			if (light.m_IsMain)
			{
				return light;
			}
		}
		for (auto entity : view)
		{
			auto& light = m_Registry.get<DirectionalLight>(entity);
			light.m_IsMain = true;
			return light;
		}
	}
	// 在场景第一帧之前调用
	void Scene::OnAwake()
	{
		// Create TimerWheel
		m_PhysicsTimerWheel =  CreateRef<PhysicsTimerWheel>(10, 1000 * Time::GetFixedTime());

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
		m_PhysicsWorld2D = CreateRef<Physics2DWorld>(Vector2(0.0f, -9.8f), this);
		// Add physical contact listener 2D
		m_PhysicalContactListener2D = CreateRef<PhysicalContactListener2D>(this);
		m_PhysicsWorld2D->SetContactListener(m_PhysicalContactListener2D.get());


		auto view = m_Registry.view<RigidBody2D>();
		for (auto rig : view)
		{
			GameObject gameObject = m_Registry.get<RigidBody2D>(rig).m_GameObject;
			auto& transform = gameObject.GetComponent<Transform>();
			auto& rigidBody = gameObject.GetComponent<RigidBody2D>();

			Physics2DBodyDef bodyDef;
			bodyDef.SetType(rigidBody.m_RigidBodyType);
			bodyDef.SetPosition({ transform.m_Position.value.x, transform.m_Position.value.y });
			bodyDef.SetAngle(transform.GetEulerAngle().value.z);

			Physics2DBody* body = m_PhysicsWorld2D->CreateBody(&bodyDef);
			body->SetFixedRotation(rigidBody.m_FixedRotation);
			rigidBody.m_Body = body;

			if (gameObject.HasComponent<BoxCollider2D>())
			{
				auto& boxCollider = gameObject.GetComponent<BoxCollider2D>();

				Physics2DPolygonShape shape; 
				shape.SetAsBox(boxCollider.m_Size.value.x / 2.0f * transform.m_Scale.value.x, boxCollider.m_Size.value.y / 2.0f * transform.m_Scale.value.y, 
					Vector2(boxCollider.m_Offset.value.x, boxCollider.m_Offset.value.y), Math::Radians(boxCollider.m_Rotation));

				Physics2DFixtureDef fixtureDef;
				fixtureDef.SetPolygonShape(&shape);
				fixtureDef.SetDensity(boxCollider.m_Density);
				fixtureDef.SetFriction(boxCollider.m_Friction);
				fixtureDef.SetRestitution(boxCollider.m_Friction);
				fixtureDef.SetRestitutionThreshold(boxCollider.m_RestitutionThreshold);
				fixtureDef.SetIsTrigger(boxCollider.m_IsTrigger);
				fixtureDef.SetGameObject(gameObject);
				body->CreateFixture(&fixtureDef);
			}
			if (gameObject.HasComponent<CircleCollider2D>())
			{
				auto& circleCollider = gameObject.GetComponent<CircleCollider2D>();

				Physics2DCircleShape shape;
				shape.SetPosition(circleCollider.m_Offset); 
				shape.SetRadius(Math::Max(transform.m_Scale) * circleCollider.m_Radius);

				Physics2DFixtureDef fixtureDef;
				fixtureDef.SetCircleShape(&shape);
				fixtureDef.SetDensity(circleCollider.m_Density);
				fixtureDef.SetFriction(circleCollider.m_Friction);
				fixtureDef.SetRestitution(circleCollider.m_Friction);
				fixtureDef.SetRestitutionThreshold(circleCollider.m_RestitutionThreshold);
				fixtureDef.SetIsTrigger(circleCollider.m_IsTrigger);
				fixtureDef.SetGameObject(gameObject);
				body->CreateFixture(&fixtureDef);
			}
		}
	}
	// 场景第一帧调用
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

		// Start timerwheel
		m_PhysicsTimerWheel->Start();
		// Add physics update
		m_PhysicsTimerWheel->AddTask(1000.0f * Time::GetFixedTime(), [=]() {
				m_PhysicsWorld2D->Step(Time::GetPhysicsDeltaTime());

				// retrieve transform
				auto view = m_Registry.view<RigidBody2D>();
				for (auto entity : view)
				{
					GameObject gameObject = m_Registry.get<RigidBody2D>(entity).m_GameObject;
					auto& transform = gameObject.GetComponent<Transform>();
					auto& rigidBody = gameObject.GetComponent<RigidBody2D>();

					Physics2DBody* body = (Physics2DBody*)rigidBody.m_Body;
					const auto& pos = body->GetPosition();
					transform.m_Position.value.x = pos.value.x;
					transform.m_Position.value.y = pos.value.y;
					transform.SetEulerAngleInRadians({ 0.0f, 0.0f, body->GetAngle() });
				}
				m_Registry.view<NativeScript>().each([=](auto entity, auto& script)
					{
						if (script.Instance)
						{
							script.Instance->OnPhysicsUpdate();
						}
						
					});
			});
		
	}
	// 场景中的所有对象在每一帧开始时调用
	// 更新物理和脚本
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
	}
	// 场景中的所有对象在每一帧结束时调用
	void Scene::OnLateUpdate()
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
					script.Instance->OnLateUpdate();
				});
		}
	}
	// 场景中的所有对象在每一帧结束后调用
	void Scene::OnEndFrame()
	{
		for(auto object : m_DeletedGameObject)
		{
			auto script = object.TryGetComponent<NativeScript>();
			if (script)
			{
				script->Instance->OnDestroy();
			}
			m_Registry.destroy(object);
		}
		m_DeletedGameObject.clear();
	}
	void Scene::OnPause()
	{
		if (m_PhysicsTimerWheel)
		{
			m_PhysicsTimerWheel->Pause();
		}
	}
	void Scene::OnResume()
	{
		if (m_PhysicsTimerWheel)
		{
			m_PhysicsTimerWheel->Continue();
		}
	}
	// 渲染场景中的所有对象
	void Scene::OnRender()
	{
		std::lock_guard<std::mutex> lock(CoreThread::s_Mutex);

		auto view_Light = m_Registry.view<Transform, DirectionalLight>();
		Vector3 main_dir = { -1.0f, -1.0f, 0.0f };
		Vector3 main_color = { 1.0f, 1.0f, 1.0f };
		DirectionalLight mainLight = MainDirectionalLight();
		if (mainLight.m_GameObject)
		{
			main_dir = Math::Rotate(mainLight.m_GameObject.GetComponent<Transform>().m_Rotation, { 0.0f, 0.0f, -1.0f });
			main_color = mainLight.m_Color;
		}
		Renderer::SetLightUniforms(main_dir, main_color);

		Renderer::SetLightUniforms(main_dir, main_color);
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
	template <>
	void GENGINE_API Scene::OnComponentAdded<DirectionalLight>(GameObject gameObject, DirectionalLight& component)
	{
	}
}