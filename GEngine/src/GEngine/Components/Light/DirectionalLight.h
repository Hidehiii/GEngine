#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Components/Component.h"

namespace GEngine
{
	// Contains the directional light of a game object
	// The directional light is used to create a light source
	class DirectionalLight : public Component
	{
	public:
		DirectionalLight() = default;
		DirectionalLight(const DirectionalLight&) = default;

	public:
		// Color of the light
		Vector4 m_Color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		// Is the light the main light
		bool m_IsMain = false;
	};
}


