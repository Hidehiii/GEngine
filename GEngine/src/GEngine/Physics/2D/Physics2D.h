#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Components/Components.h"
#include "GEngine/Physics/2D/PhysicalContactListener2D.h"
#include "GEngine/Object/GameObject.h"

// Box2D
#include <box2d/include/box2d/b2_world.h>
#include <box2d/include/box2d/b2_body.h>
#include <box2d/include/box2d/b2_fixture.h>
#include <box2d/include/box2d/b2_polygon_shape.h>
#include <box2d/include/box2d/b2_circle_shape.h>


namespace GEngine
{
	class Physics2DBodyDef;
	class Physics2DBody;
	class Physics2DFixtureDef;
	class Physics2DPolygonShape;
	class Physics2DCircleShape;
	class Physics2DFixture;
	class Scene;


	class GENGINE_API Physics2DWorld
	{
	public:
		Physics2DWorld(Vector2 gravity, Scene* scene);

		Physics2DBody* CreateBody(const Physics2DBodyDef* bodyDef);
		void Simulate(float timeStep);
		void SetContactListener(PhysicalContactListener2D* listener);

		~Physics2DWorld();

		uint32_t m_VelocityIterations = 6;
		uint32_t m_PositionIterations = 2;
	private:
		b2World* m_PhysicsWorld2D;
		Scene* m_Scene = nullptr;
	};

	class GENGINE_API Physics2DBodyDef
	{
	public:
		void SetType(RigidBody2D::RigidBodyType type);
		void SetPosition(Vector2 position);
		void SetAngle(float angle);
	private:
		b2BodyDef m_BodyDef;
		friend class Physics2DWorld;
	};

	class GENGINE_API Physics2DBody
	{
	public:
		Physics2DBody() = default;
		Physics2DBody(b2Body* body)
			: m_Body(body) {}

		~Physics2DBody() = default;

		void SetFixedRotation(bool fixedRotation);

		void CreateFixture(Physics2DFixtureDef* fixtureDef);

		Vector2 GetPosition() { return { this->m_Body->GetPosition().x, this->m_Body->GetPosition().y }; }
		float GetAngle() { return this->m_Body->GetAngle(); }
	private:
		b2Body* m_Body = nullptr;
		friend class Physics2DWorld;
	};

	class GENGINE_API Physics2DFixtureDef
	{
	public:
		void SetPolygonShape(Physics2DPolygonShape* shape);
		void SetCircleShape(Physics2DCircleShape* shape);

		void SetDensity(float density);
		void SetFriction(float friction);
		void SetRestitution(float restitution);
		void SetRestitutionThreshold(float restitutionThreshold);
		void SetIsTrigger(bool isTrigger);
		void SetGameObject(GameObject gameObject);
	private:
		b2FixtureDef m_FixtureDef;
		friend class Physics2DBody;
	};

	class GENGINE_API Physics2DFixture
	{
	public:
		Physics2DFixture() = default;
		GameObject GetGameObject();
	private:
		GameObject m_GameObject;
		b2Fixture* m_Fixture = nullptr;
		friend class Physics2DBody;
		friend class Physics2DContactInfo;
	};

	class GENGINE_API Physics2DPolygonShape
	{
	public:

		void SetAsBox(float hx, float hy, const Vector2& center, float angle);
		void SetAsBox(float hx, float hy);
	private:
		b2PolygonShape m_Shape;
		friend class Physics2DFixtureDef;
	};

	class GENGINE_API Physics2DCircleShape
	{
	public:
		void SetPosition(Vector2 pos);
		void SetRadius(float radius);
	private:
		b2CircleShape m_Shape;
		friend class Physics2DFixtureDef;
	};
}



