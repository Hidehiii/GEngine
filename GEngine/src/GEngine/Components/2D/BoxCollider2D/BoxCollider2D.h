#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Components/Component.h"

namespace GEngine
{
	class GENGINE_API BoxCollider2D : public Component
	{
	public:
		BoxCollider2D() = default;
		BoxCollider2D(Vector2 offset, Vector2 size)
			: m_Offset(offset), m_Size(size) {}
		BoxCollider2D(const BoxCollider2D&) = default;
	public:
		// This transform is base on local space
		// The offset is the center of the box collider
		// If you want to switch this to the world space, you need to multiply the transform matrix by the following order
		// T transform, R rotation, S scale
		// T_Model * R_Model * T_Collider * R_Collider * S_Model * S_Collider
		Vector2 m_Offset = { 0.0f, 0.0f };
		Vector2 m_Size = { 1.0f, 1.0f };
		float m_Rotation = 0.0f;

		float m_Density = 1.0f;
		float m_Friction = 0.0f;
		float m_Restitution = 0.0f;
		float m_RestitutionThreshold = 0.5f;

		bool m_IsTrigger = false;
	};
}


