#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Math/Vector3.h"
#include "GEngine/Object/GameObject.h"

namespace GEngine
{
	class AABB
	{
	public:
		AABB() = default;
		AABB(const AABB&) = default;

		AABB(const Vector3& min, const Vector3& max)
			: m_Min(min), m_Max(max) {}

		AABB(GameObject& gameObject)
		{
			CalculateAABB(gameObject);
		}

		// Get the minimum point of the AABB
		Vector3 Min() const { return m_Min; }
		// Get the maximum point of the AABB
		Vector3 Max() const { return m_Max; }

		float LengthX() const { return m_Max.x - m_Min.x; }
		float LengthY() const { return m_Max.y - m_Min.y; }
		float LengthZ() const { return m_Max.z - m_Min.x; }
	private:
		void CalculateAABB(GameObject& gameObject);
	private:
		// Minimum point of the AABB
		Vector3 m_Min = Vector3(0.0f, 0.0f, 0.0f);
		// Maximum point of the AABB
		Vector3 m_Max = Vector3(0.0f, 0.0f, 0.0f);
	};
}


