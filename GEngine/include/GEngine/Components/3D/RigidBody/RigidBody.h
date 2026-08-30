#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Components/Component.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Core/AABB.h"

#include "PhysX/include/PxPhysicsAPI.h"

namespace GEngine
{
	class GENGINE_API RigidBody : public Component
	{
	public:
		enum class RigidBodyType
		{
			// The rigid body is static
			Static = 0,
			// The rigid body is dynamic
			// The rigid body is affected by forces and collisions
			Dynamic = 1,
			// The rigid body is kinematic
			Kinematic = 2
		};
	public:
		void SetMass(float mass);
		float GetMass();
	private:
		// The PhysX body of the rigid body
		void* m_Body = nullptr;
		friend class Scene;
	};
}



