#include "GEpch.h"
#include "Scene.h"
#include "GEngine/Core/Time.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Components/Components.h"
#include "GEngine/Object/GameObject.h"
#include "GEngine/Physics/2D/Physics2D.h"
#include "GEngine/Physics/2D/PhysicalContactListener2D.h"
#include "GEngine/Physics/3D/Physics3D.h"


namespace GEngine
{
	template<typename... Component>
	static void CallComponentFunction(GameObject& obj, ComponentFunction function)
	{
		([&]()
			{
				auto component = obj.TryGetComponent<Component>();
				if (component)
				{
					switch (function)
					{
					case ComponentFunction::OnAwake:		component->OnAwake(); break;
					case ComponentFunction::OnStart:		component->OnStart(); break;
					case ComponentFunction::OnUpdate:		component->OnUpdate(); break;
					case ComponentFunction::OnPhysicsUpdate:component->OnPhysicsUpdate(); break;
					case ComponentFunction::OnLateUpdate:	component->OnLateUpdate(); break;
					case ComponentFunction::OnDestroy:		component->OnDestroy(); break;
					case ComponentFunction::OnRender:		component->OnRender(); break;
					default:
						break;
					}
				}
			}(), ...);
	}
	template<typename... Component>
	static void CallComponentFunction(ComponentGroup<Component...>, GameObject& obj, ComponentFunction function)
	{
		CallComponentFunction<Component...>(obj, function);
	}
	template<typename... Component>
	static void CallComponentFunction(entt::registry& reg, ComponentFunction function)
	{
		([&]()
			{
				auto view = reg.view<Component>();
				for (auto entity : view)
				{
					switch (function)
					{
					case ComponentFunction::OnAwake:		reg.get<Component>(entity).OnAwake(); break;
					case ComponentFunction::OnStart:		reg.get<Component>(entity).OnStart(); break;
					case ComponentFunction::OnUpdate:		reg.get<Component>(entity).OnUpdate(); break;
					case ComponentFunction::OnPhysicsUpdate:reg.get<Component>(entity).OnPhysicsUpdate(); break;
					case ComponentFunction::OnLateUpdate:	reg.get<Component>(entity).OnLateUpdate(); break;
					case ComponentFunction::OnDestroy:		reg.get<Component>(entity).OnDestroy(); break;
					case ComponentFunction::OnRender:		reg.get<Component>(entity).OnRender(); break;
					default:
						break;
					}
				}
			}(), ...);
	}
	template<typename... Component>
	static void CallComponentFunction(ComponentGroup<Component...>, entt::registry& reg, ComponentFunction function)
	{
		CallComponentFunction<Component...>(reg, function);
	}
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
					GameObject obj = { dstEntity, scene.get() };
					obj.AddOrReplaceComponent<Component>(srcComponent);
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
		std::lock_guard<std::mutex> lock(CORE_THREAD_MUTEX);
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
		object.AddComponent<NativeScript>();
		return object;
	}
	void Scene::DuplicateGameObject(GameObject gameObject)
	{
		GameObject newObject = CreateGameObject(gameObject.GetComponent<Attribute>().m_Name);
		UUID newUuid = newObject.GetComponent<Attribute>().m_UUID;
		CopyComponentIfExists(AllComponents{}, newObject, gameObject);
		newObject.GetComponent<Attribute>().m_UUID = newUuid;
	}
	void Scene::DestroyGameObject(GameObject gameObject)
	{
		m_DeletedGameObject.push_back(gameObject);
	}
	Camera* Scene::MainCamera()
	{
		auto view = m_Registry.view<Camera>();
		if (view.size() == 0)
		{
			return nullptr;
		}
		for (auto entity : view)
		{
			auto& camera = m_Registry.get<Camera>(entity);
			if (camera.isPrimary)
			{
				return &camera;
			}
		}
		for (auto entity : view)
		{
			auto& camera = m_Registry.get<Camera>(entity);
			camera.isPrimary = true;
			return &camera;
		}
	}
	DirectionalLight* Scene::MainDirectionalLight()
	{
		auto view = m_Registry.view<DirectionalLight>();
		if (view.size() == 0)
		{
			return nullptr;
		}
		for (auto entity : view)
		{
			auto& light = m_Registry.get<DirectionalLight>(entity);
			if (light.m_IsMain)
			{
				return &light;
			}
		}
		for (auto entity : view)
		{
			auto& light = m_Registry.get<DirectionalLight>(entity);
			light.m_IsMain = true;
			return &light;
		}
	}
	// 在场景第一帧之前调用
	void Scene::OnAwake()
	{
		// Create TimerWheel
		m_PhysicsTimerWheel =  CreateRef<PhysicsTimerWheel>(10, 1000 * Time::GetFixedTime());

		CallComponentFunction(AllComponents{}, m_Registry, ComponentFunction::OnAwake);
		// Add physics world 2D
		m_PhysicsWorld2D = CreateRef<Physics2DWorld>(Vector2(0.0f, -9.8f), this);
		// Add physical contact listener 2D
		m_PhysicalContactListener2D = CreateRef<PhysicalContactListener2D>(this);
		m_PhysicsWorld2D->SetContactListener(m_PhysicalContactListener2D.get());

		// 3d
		m_PhysicsWorld3D = CreateRef<Physics3DWorld>();


		auto view = m_Registry.view<RigidBody2D>();
		for (auto rig : view)
		{
			GameObject gameObject = m_Registry.get<RigidBody2D>(rig).m_GameObject;
			auto& transform = gameObject.GetComponent<Transform>();
			auto& rigidBody = gameObject.GetComponent<RigidBody2D>();

			Physics2DBodyDef bodyDef;
			bodyDef.SetType(rigidBody.m_RigidBodyType);
			bodyDef.SetPosition({ transform.m_Position.x, transform.m_Position.y });
			bodyDef.SetAngle(transform.GetEulerAngle().z);

			Physics2DBody* body = m_PhysicsWorld2D->CreateBody(&bodyDef);
			body->SetFixedRotation(rigidBody.m_FixedRotation);
			rigidBody.m_Body = body;

			if (gameObject.HasComponent<BoxCollider2D>())
			{
				auto& boxCollider = gameObject.GetComponent<BoxCollider2D>();

				Physics2DPolygonShape shape; 
				shape.SetAsBox(boxCollider.m_Size.x / 2.0f * transform.m_Scale.x, boxCollider.m_Size.y / 2.0f * transform.m_Scale.y, 
					Vector2(boxCollider.m_Offset.x, boxCollider.m_Offset.y), Math::Radians(boxCollider.m_Rotation));

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

		CallComponentFunction(AllComponents{}, m_Registry, ComponentFunction::OnStart);

		// Start timerwheel
		m_PhysicsTimerWheel->Start();
		// Add physics update
		m_PhysicsTimerWheel->AddTask(1000.0f * Time::GetFixedTime(), [&]() {
				m_PhysicsWorld2D->Simulate(Time::GetPhysicsDeltaTime());
				//m_PhysicsWorld3D->Simulate(Time::GetPhysicsDeltaTime());

				// retrieve transform
				auto view = m_Registry.view<RigidBody2D>();
				for (auto entity : view)
				{
					GameObject gameObject = m_Registry.get<RigidBody2D>(entity).m_GameObject;
					auto& transform = gameObject.GetComponent<Transform>();
					auto& rigidBody = gameObject.GetComponent<RigidBody2D>();

					Physics2DBody* body = (Physics2DBody*)rigidBody.m_Body;
					const auto& pos = body->GetPosition();
					transform.m_Position.x = pos.x;
					transform.m_Position.y = pos.y;
					transform.SetEulerAngleInRadians({ 0.0f, 0.0f, body->GetAngle() });
				}

				CallComponentFunction(AllComponents{}, m_Registry, ComponentFunction::OnPhysicsUpdate);
			});
		
	}
	// 场景中的所有对象在每一帧开始时调用
	// 更新物理和脚本
	void Scene::OnUpdate()
	{
		CallComponentFunction(AllComponents{}, m_Registry, ComponentFunction::OnUpdate);
	}
	// 场景中的所有对象在每一帧结束时调用
	void Scene::OnLateUpdate()
	{
		CallComponentFunction(AllComponents{}, m_Registry, ComponentFunction::OnLateUpdate);
	}
	// 场景中的所有对象在每一帧结束后调用
	void Scene::OnEndFrame()
	{
		for(auto object : m_DeletedGameObject)
		{
			auto nativeScript = object.TryGetComponent<NativeScript>();
			if (nativeScript)
			{
				nativeScript->OnDestroy();
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


	bool ObjSort(GameObject& a, GameObject& b)
	{
		/*return Math::Distance(Graphics::GetRenderTargetCameraPosition(), a.GetComponent<Transform>().m_Position) >
			Math::Distance(Graphics::GetRenderTargetCameraPosition(), b.GetComponent<Transform>().m_Position);*/
		GE_CORE_ASSERT(false, "");
		return false;
	}
	// 渲染场景中的所有对象
	void Scene::OnRender()
	{
		std::lock_guard<std::mutex> lock(CORE_THREAD_MUTEX);

		//CallComponentFunction(AllComponents{}, m_Registry, ComponentFunction::OnRender);
		// 距离排序可能要
		// TODO
		auto e = m_Registry.view<Attribute>();
		std::vector<GameObject> entities;
		for (auto it = e.begin(); it != e.end(); it++)
		{
			entities.push_back({ *it, this });
		}
		std::sort(entities.begin(), entities.end(), ObjSort);
		for (auto it = entities.begin(); it != entities.end(); it++)
		{
			CallComponentFunction(AllComponents{}, *it, ComponentFunction::OnRender);
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

}