#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Components/Components.h"
#include <filesystem>

#include <PhysX/include/PxPhysicsAPI.h>





namespace GEngine
{
	class PhysicsAllocatorCallBack3D;
	class PhysicsErrorCallback3D;


	class GENGINE_API Physics3D
	{
	public:
		static void Init();
		static void Shutdown();

	public:
		static PhysicsAllocatorCallBack3D s_AllocatorCallback;
		static PhysicsErrorCallback3D s_ErrorCallback;

	private:
		static physx::PxFoundation* s_Foundation;
		static physx::PxPhysics* s_Physics;
		static physx::PxDefaultCpuDispatcher* s_Dispatcher;

		friend class Physics3DWorld;
		friend class Physics3DBody;
	};

	class GENGINE_API PhysicsAllocatorCallBack3D : public physx::PxAllocatorCallback
	{
	public:
		virtual ~PhysicsAllocatorCallBack3D() override {}

		virtual void* allocate(size_t size, const char* typeName, const char* filename, int line) override
		{
#ifdef GE_PLATFORM_WINDOWS
			return _aligned_malloc(size, 16);
#else
			return malloc(size);
#endif
		}

		virtual void deallocate(void* ptr) override
		{
#ifdef GE_PLATFORM_WINDOWS
			_aligned_free(ptr);
#else
			free(ptr);
#endif
		}
	};

	class GENGINE_API PhysicsErrorCallback3D : public physx::PxErrorCallback
	{
	public:
		virtual ~PhysicsErrorCallback3D() override {}

		virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override
		{
			switch (code)
			{
			case physx::PxErrorCode::eDEBUG_INFO:
				GE_CORE_INFO("PhysX Debug Info: {0} in {1} at line {2}", message, file, line);
				break;
			case physx::PxErrorCode::eDEBUG_WARNING:
				GE_CORE_WARN("PhysX Debug Warning: {0} in {1} at line {2}", message, file, line);
				break;
			case physx::PxErrorCode::eINVALID_PARAMETER:
				GE_CORE_ERROR("PhysX Invalid Parameter: {0} in {1} at line {2}", message, file, line);
				break;
			case physx::PxErrorCode::eINVALID_OPERATION:
				GE_CORE_ERROR("PhysX Invalid Operation: {0} in {1} at line {2}", message, file, line);
				break;
			case physx::PxErrorCode::eOUT_OF_MEMORY:
				GE_CORE_ERROR("PhysX Out of Memory: {0} in {1} at line {2}", message, file, line);
				break;
			case physx::PxErrorCode::eINTERNAL_ERROR:
				GE_CORE_ERROR("PhysX Internal Error: {0} in {1} at line {2}", message, file, line);
				break;
			case physx::PxErrorCode::eABORT:
				GE_CORE_ERROR("PhysX Abort: {0} in {1} at line {2}", message, file, line);
				break;
			default:
				GE_CORE_ERROR("PhysX Unknown Error: {0} in {1} at line {2}", message, file, line);
				break;
			}
		}
	};

	class GENGINE_API Physics3DSphere
	{
	public:
		Physics3DSphere(float radius = 0);
		Physics3DSphere(const Physics3DSphere& other);

		bool IsValid() const;

		inline void operator=(const Physics3DSphere& other);

	public:
		float m_Radius;

	private:
		physx::PxSphereGeometry m_Sphere;
	};

	class GENGINE_API Physics3DCapsule
	{
	public:
		Physics3DCapsule(float radius = 0.0f, float halfHeight = 0.0f);
		Physics3DCapsule(const Physics3DCapsule& other);

		bool IsValid() const;

		inline void operator=(const Physics3DCapsule& other);

	public:
		float m_Radius;
		float m_HalfHeight;

	private:
		physx::PxCapsuleGeometry m_Capsule;
	};

	class GENGINE_API Physics3DCube
	{
	public:
		Physics3DCube(float hx = 0, float hy = 0, float hz = 0);
		Physics3DCube(const Vector3& halfExtents);
		Physics3DCube(const Physics3DCube& other);

		bool IsValid() const;

		inline void operator=(const Physics3DCube& other);
	public:
		Vector3 m_HalfExtents;
	private:
		physx::PxBoxGeometry m_Cube;
	};

	class GENGINE_API Physics3DPlane
	{
	public:
		Physics3DPlane();
		Physics3DPlane(const Physics3DPlane& other);

		bool IsValid() const;

		inline void operator=(const Physics3DPlane& other);
	private:
		physx::PxPlaneGeometry m_Plane;
	};

	class GENGINE_API Physics3DBody
	{
	public:
		Physics3DBody(RigidBody::RigidBodyType type);

		void SetType(RigidBody::RigidBodyType type);
	private:
		physx::PxRigidDynamic*	m_BodyDynamic = nullptr;
		physx::PxRigidStatic*	m_BodyStatic = nullptr;
		RigidBody::RigidBodyType m_Type;
	};


	class GENGINE_API Physics3DWorld
	{
	public:
		Physics3DWorld(Vector3 gravity = { 0.0f, -9.81f, 0.0f }); 

		void Simulate(float timeStep);
	private:
		physx::PxScene* m_Scene;
	};
}



