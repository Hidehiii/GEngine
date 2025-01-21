#include "GEpch.h"
#include "Physics3D.h"

namespace GEngine
{
	PhysicsAllocatorCallBack3D		Physics3D::s_AllocatorCallback;
	PhysicsErrorCallback3D			Physics3D::s_ErrorCallback;
	physx::PxFoundation*			Physics3D::s_Foundation			= nullptr;
	physx::PxPhysics*				Physics3D::s_Physics			= nullptr;
	physx::PxDefaultCpuDispatcher*	Physics3D::s_Dispatcher			= nullptr;


	void Physics3D::Init()
	{
		s_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_AllocatorCallback, s_ErrorCallback);
		GE_CORE_ASSERT(s_Foundation, "Failed to create PhysX Foundation");

		s_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *s_Foundation, physx::PxTolerancesScale());
		GE_CORE_ASSERT(s_Physics, "Failed to create PhysX Physics");
		
	}
	void Physics3D::Shutdown()
	{
		s_Physics->release();
		s_Foundation->release();
	}


	Physics3DSphere::Physics3DSphere(float radius)
	{
		m_Radius = radius;
		m_Sphere = physx::PxSphereGeometry(radius);
	}

	Physics3DSphere::Physics3DSphere(const Physics3DSphere& other)
	{
		m_Radius = other.m_Radius;
		m_Sphere = physx::PxSphereGeometry(other.m_Sphere);
	}

	bool Physics3DSphere::IsValid() const
	{
		return m_Sphere.isValid();
	}

	inline void Physics3DSphere::operator=(const Physics3DSphere& other)
	{
		this->m_Sphere = other.m_Sphere;
		this->m_Radius = other.m_Radius;
	}

	Physics3DCapsule::Physics3DCapsule(float radius, float halfHeight)
	{
		m_Radius = radius;
		m_HalfHeight = halfHeight;
		m_Capsule = physx::PxCapsuleGeometry(radius, halfHeight);
	}

	Physics3DCapsule::Physics3DCapsule(const Physics3DCapsule& other)
	{
		m_Radius = other.m_Radius;
		m_HalfHeight = other.m_HalfHeight;
		m_Capsule = physx::PxCapsuleGeometry(other.m_Capsule);
	}

	bool Physics3DCapsule::IsValid() const
	{
		return m_Capsule.isValid();
	}

	inline void Physics3DCapsule::operator=(const Physics3DCapsule& other)
	{
		this->m_Capsule = other.m_Capsule;
		this->m_Radius = other.m_Radius;
		this->m_HalfHeight = other.m_HalfHeight;
	}


	Physics3DCube::Physics3DCube(float hx, float hy, float hz)
	{
		m_HalfExtents = Vector3(hx, hy, hz);
		m_Cube = physx::PxBoxGeometry(hx, hy, hz);
	}
	Physics3DCube::Physics3DCube(const Vector3& halfExtents)
	{
		m_HalfExtents = halfExtents;
		m_Cube = physx::PxBoxGeometry(halfExtents.x, halfExtents.y, halfExtents.z);
	}
	Physics3DCube::Physics3DCube(const Physics3DCube& other)
	{
		m_HalfExtents = other.m_HalfExtents;
		m_Cube = physx::PxBoxGeometry(other.m_Cube);
	}
	bool Physics3DCube::IsValid() const
	{
		return m_Cube.isValid();
	}
	inline void Physics3DCube::operator=(const Physics3DCube& other)
	{
		this->m_HalfExtents = other.m_HalfExtents;
		this->m_Cube = other.m_Cube;
	}
	Physics3DPlane::Physics3DPlane()
	{
		m_Plane = physx::PxPlaneGeometry();
	}
	Physics3DPlane::Physics3DPlane(const Physics3DPlane& other)
	{
		m_Plane = physx::PxPlaneGeometry(other.m_Plane);
	}
	bool Physics3DPlane::IsValid() const
	{
		return m_Plane.isValid();
	}
	inline void Physics3DPlane::operator=(const Physics3DPlane& other)
	{
		this->m_Plane = other.m_Plane;
	}
	Physics3DBody::Physics3DBody(RigidBody::RigidBodyType type)
	{
		m_Type = type;
		switch (type)
		{
		case RigidBody::RigidBodyType::Static:
			m_BodyStatic = Physics3D::s_Physics->createRigidStatic({ 0, 0, 0 });
			break;
		case RigidBody::RigidBodyType::Dynamic:
			m_BodyDynamic = Physics3D::s_Physics->createRigidDynamic({ 0, 0, 0 });
			break;
		case RigidBody::RigidBodyType::Kinematic:
			m_BodyDynamic = Physics3D::s_Physics->createRigidDynamic({ 0, 0, 0 });
			break;
		default:
			break;
		}
	}
	void Physics3DBody::SetType(RigidBody::RigidBodyType type)
	{
		m_Type = type;
		switch (type)
		{
		case RigidBody::RigidBodyType::Static:
			m_BodyStatic = Physics3D::s_Physics->createRigidStatic({ 0, 0, 0 });
			break;
		case RigidBody::RigidBodyType::Dynamic:
			m_BodyDynamic = Physics3D::s_Physics->createRigidDynamic({ 0, 0, 0 });
			break;
		case RigidBody::RigidBodyType::Kinematic:
			m_BodyDynamic = Physics3D::s_Physics->createRigidDynamic({ 0, 0, 0 });
			break;
		default:
			break;
		}
	}
	Physics3DWorld::Physics3DWorld(Vector3 gravity)
	{
		physx::PxSceneDesc	desc(Physics3D::s_Physics->getTolerancesScale());
		desc.gravity		= physx::PxVec3(gravity.x, gravity.y, gravity.z);	

		m_Scene				= Physics3D::s_Physics->createScene(desc);
	}
	void Physics3DWorld::Simulate(float timeStep)
	{
		m_Scene->simulate(timeStep);
	}
}