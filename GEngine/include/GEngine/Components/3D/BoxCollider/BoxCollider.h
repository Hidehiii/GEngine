#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Components/Component.h"


namespace GEngine {
	class GENGINE_API BoxCollider : public Component
	{
	public:
		BoxCollider() = default;
		BoxCollider(Vector3 size, Vector3 offset = {0, 0, 0}, Vector3 rotation = {0, 0, 0})
			: m_Size(size), m_Offset(offset), m_Rotation(rotation) {}

	public:
		Vector3 m_Size;
		Vector3 m_Offset;
		Vector3 m_Rotation;
	};
}



