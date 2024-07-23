#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Object/GameObject.h"

#include <box2d/include/box2d/b2_world_callbacks.h>



namespace GEngine
{
	class PhysicalContactListener2D;
	class Physics2DFixture;
	class Scene;

	class GENGINE_API Physics2DContactInfo
	{
	public:
		Physics2DContactInfo() = default;

		GameObject GetOtherGameObject();
	private:
		Ref<Physics2DFixture> GetFixtureA();
		Ref<Physics2DFixture> GetFixtureB();
		GameObject m_OtherGameObject;
		b2Contact* m_Contact = nullptr;
		Scene* m_Scene = nullptr;
		friend class PhysicalContactListener2D;
	};

	class GENGINE_API PhysicalContactListener2D : public b2ContactListener
	{
	public:
		PhysicalContactListener2D(Scene* scene)
			: m_Scene(scene) {}

		virtual void BeginContact(b2Contact* contact);
		virtual void EndContact(b2Contact* contact);
		virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);
		virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse);

		// 分发事件到具体的物理组件和脚本
		virtual void OnContactBegin(Ref<Physics2DContactInfo> info);
		virtual void OnContactEnd(Ref<Physics2DContactInfo> info);
	private:
		Scene* m_Scene = nullptr;
	};

}


