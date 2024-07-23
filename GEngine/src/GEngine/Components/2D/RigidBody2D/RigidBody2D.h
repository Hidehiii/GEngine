#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Components/Component.h"
#include "GEngine/Math/Math.h"

namespace GEngine
{
	// Contains the rigid body of a game object
	// The rigid body is used to create a physics body
	class GENGINE_API RigidBody2D : public Component
	{
	public:
		enum  class RigidBodyType
		{
			// The rigid body is static
			Static = 0,
			// The rigid body is dynamic
			// The rigid body is affected by forces and collisions
			Dynamic = 1,
			// The rigid body is kinematic
			Kinematic = 2
		};

	static std::string RigidBodyTypeToString(RigidBody2D::RigidBodyType type)
	{
		switch (type)
		{
		case RigidBody2D::RigidBodyType::Static: return "Static";
		case RigidBody2D::RigidBodyType::Dynamic: return "Dynamic";
		case RigidBody2D::RigidBodyType::Kinematic: return "Kinematic";
		default:
			break;
		}
		GE_CORE_ASSERT(false, "Unknown Type");
		return std::string();
	}

	public:
		RigidBody2D() = default;
		RigidBody2D(RigidBody2D::RigidBodyType type)
			: m_RigidBodyType(type) {}
		RigidBody2D(const RigidBody2D&) = default;
	public:
		// The type of the rigid body
		RigidBodyType m_RigidBodyType = RigidBodyType::Static;
		// Is the rigid body could be rotated
		// This would only affect Z rotation
		bool m_FixedRotation = false;
		// The box2d body of the rigid body
		void* m_Body = nullptr;
	};


	
}


