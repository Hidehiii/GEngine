#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Components/Component.h"
#include "GEngine/Math/Math.h"
namespace GEngine
{
	// Contains the circle collider of a game object
	// The circle collider is used to create a circle collider
	class GENGINE_API CircleCollider2D : public Component
	{
	public:
		CircleCollider2D() = default;
		CircleCollider2D(Vector2 offset, float radius)
			: m_Offset(offset), m_Radius(radius) {}
		CircleCollider2D(const CircleCollider2D&) = default;

	public:
		// Offset of the circle collider
		Vector2 m_Offset = { 0.0f, 0.0f };
		// Radius of the circle collider
		float  m_Radius = 0.5f;

		// Physics material of the circle collider
		float m_Density = 1.0f;
		float m_Friction = 0.0f;
		float m_Restitution = 0.0f;
		float m_RestitutionThreshold = 0.5f;

		// Is the circle collider a trigger
		bool m_IsTrigger = false;
	};
}


