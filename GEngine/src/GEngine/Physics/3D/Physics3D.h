#pragma once

#include "GEngine/Core/Core.h"

#include <filesystem>

#include <PxPhysicsAPI.h>
#include <PxPhysics.h>
#include <foundation/PxAllocatorCallback.h>
#include <pvd/PxPvd.h>
#include <cooking/PxCooking.h>


namespace GEngine
{
	class GENGINE_API Physics3D
	{
	public:
		void Init(std::filesystem::path pvdFilePath, bool recordMemoryAllocations = true);
		void Shutdown();
	public:
		static PhysicsAllocatorCallBack3D s_AllocatorCallback;
		static PhysicsErrorCallback3D s_ErrorCallback;

	private:
		physx::PxFoundation* m_Foundation = nullptr;
		physx::PxPvd* m_Pvd = nullptr;
		physx::PxPhysics* m_Physics = nullptr;
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
			free(ptr);
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
}



