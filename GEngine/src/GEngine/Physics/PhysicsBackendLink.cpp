#include "GEpch.h"
#include "GEngine/Physics/2D/Physics2D.h"
#include "GEngine/Physics/2D/PhysicalContactListener2D.h"
#include "GEngine/Physics/3D/Physics3D.h"

#ifdef _MSC_VER
#pragma comment(linker, "/include:??0Physics2DWorld@GEngine@@QEAA@U?$vec@$01M$0A@@glm@@PEAVScene@1@@Z")
#pragma comment(linker, "/include:?GetOtherGameObject@Physics2DContactInfo@GEngine@@QEAA?AVGameObject@2@XZ")
#pragma comment(linker, "/include:?Init@Physics3D@GEngine@@SAXXZ")
#endif

namespace GEngine
{
	void LinkPhysicsBackends()
	{
	}
}
