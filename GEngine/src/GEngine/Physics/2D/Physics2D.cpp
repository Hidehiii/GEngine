#include "GEpch.h"
#include "Physics2D.h"
#include "GEngine/Object/Scene/Scene.h"


namespace GEngine
{
	namespace Utils
	{
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
	}

	Physics2DWorld::Physics2DWorld(Vector2 gravity, Scene* scene)
	{
		this->m_PhysicsWorld2D = new b2World(b2Vec2(gravity.x, gravity.y));
		this->m_Scene = scene;
	}

	Physics2DBody* GEngine::Physics2DWorld::CreateBody(const Physics2DBodyDef* bodyDef)
	{
		Physics2DBody* body = new Physics2DBody(this->m_PhysicsWorld2D->CreateBody(&bodyDef->m_BodyDef));
		return body;
	}

	void GEngine::Physics2DWorld::Step(float timeStep)
	{
		this->m_PhysicsWorld2D->Step(timeStep, m_VelocityIterations, m_PositionIterations);
	}

	void Physics2DWorld::SetContactListener(PhysicalContactListener2D* listener)
	{
		this->m_PhysicsWorld2D->SetContactListener(listener);
	}

	Physics2DWorld::~Physics2DWorld()
	{
		delete this->m_PhysicsWorld2D;
	}

	void Physics2DBodyDef::SetType(RigidBody2D::RigidBodyType type)
	{
		this->m_BodyDef.type = Utils::RigidBodyTypeToBox2D(type);
	}

	void Physics2DBodyDef::SetPosition(Vector2 position)
	{
		this->m_BodyDef.position.Set(position.x, position.y);
	}

	void Physics2DBodyDef::SetAngle(float angle)
	{
		this->m_BodyDef.angle = angle;
	}

	void Physics2DBody::SetFixedRotation(bool fixedRotation)
	{
		this->m_Body->SetFixedRotation(fixedRotation);
	}

	void Physics2DBody::CreateFixture(Physics2DFixtureDef* fixtureDef)
	{
		this->m_Body->CreateFixture(&fixtureDef->m_FixtureDef);
	}

	void Physics2DPolygonShape::SetAsBox(float hx, float hy, const Vector2& center, float angle)
	{
		this->m_Shape.SetAsBox(hx, hy, b2Vec2(center.x, center.y), angle);
	}

	void Physics2DPolygonShape::SetAsBox(float hx, float hy)
	{
		this->m_Shape.SetAsBox(hx, hy);
	}

	void Physics2DCircleShape::SetPosition(Vector2 pos)
	{
		this->m_Shape.m_p.Set(pos.x, pos.y);
	}
	void Physics2DCircleShape::SetRadius(float radius)
	{
		this->m_Shape.m_radius = radius;
	}

	void Physics2DFixtureDef::SetPolygonShape(Physics2DPolygonShape* shape)
	{
		this->m_FixtureDef.shape = &shape->m_Shape;
	}

	void Physics2DFixtureDef::SetCircleShape(Physics2DCircleShape* shape)
	{
		this->m_FixtureDef.shape = &shape->m_Shape;
	}

	void Physics2DFixtureDef::SetDensity(float density)
	{
		this->m_FixtureDef.density = density;
	}

	void Physics2DFixtureDef::SetFriction(float friction)
	{
		this->m_FixtureDef.friction = friction;
	}

	void Physics2DFixtureDef::SetRestitution(float restitution)
	{
		this->m_FixtureDef.restitution = restitution;
	}

	void Physics2DFixtureDef::SetRestitutionThreshold(float restitutionThreshold)
	{
		this->m_FixtureDef.restitutionThreshold = restitutionThreshold;
	}

	void Physics2DFixtureDef::SetIsTrigger(bool isTrigger)
	{
		this->m_FixtureDef.isSensor = isTrigger;
	}
	void Physics2DFixtureDef::SetGameObject(GameObject gameObject)
	{
		this->m_FixtureDef.userData.pointer = gameObject;
	}
	GameObject Physics2DFixture::GetGameObject()
	{
		return this->m_GameObject;
	}
}


