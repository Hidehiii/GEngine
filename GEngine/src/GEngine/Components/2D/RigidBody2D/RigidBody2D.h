#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Components/Component.h"
#include "GEngine/Math/Math.h"

namespace GEngine
{
	
	class GENGINE_API RigidBody2D : public Component
	{
	public:
		enum  class RigidBodyType
		{
			Static = 0,
			Dynamic = 1,
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
		RigidBodyType m_RigidBodyType = RigidBodyType::Static;
		bool m_FixedRotation = false;
		void* m_Body = nullptr;
	};


	
}


