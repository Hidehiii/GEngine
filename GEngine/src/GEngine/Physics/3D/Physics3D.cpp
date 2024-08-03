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

		m_Pvd = physx::PxCreatePvd(*m_Foundation);
		physx::PxPvdTransport* transport = physx::PxDefaultPvdFileTransportCreate(pvdFilePath.string().c_str());
		m_Pvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

		m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, physx::PxTolerancesScale(), recordMemoryAllocations, m_Pvd);
		GE_CORE_ASSERT(m_Physics, "Failed to create PhysX Physics");
	
		/*physx::PxCreateCooking*/
	}
	void Physics3D::Shutdown()
	{
		m_Pvd->release();
		m_Physics->release();
		m_Foundation->release();
	}
}