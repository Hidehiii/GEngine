#include "GEpch.h"
#include "Physics3D.h"

namespace GEngine
{
	PhysicsAllocatorCallBack3D Physics3D::s_AllocatorCallback;
	PhysicsErrorCallback3D Physics3D::s_ErrorCallback;


	void Physics3D::Init(std::filesystem::path pvdFilePath, bool recordMemoryAllocations)
	{
		m_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_AllocatorCallback, s_ErrorCallback);
		GE_CORE_ASSERT(m_Foundation, "Failed to create PhysX Foundation");

		m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, physx::PxTolerancesScale());
		GE_CORE_ASSERT(m_Physics, "Failed to create PhysX Physics");

	}
	void Physics3D::Shutdown()
	{
		m_Physics->release();
		m_Foundation->release();
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
		m_Cube = physx::PxBoxGeometry(halfExtents.value.x, halfExtents.value.y, halfExtents.value.z);
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
}