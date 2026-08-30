#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Core/UUID.h"
#include "GEngine/Components/Component.h"

namespace GEngine
{
	// Contains the base attributes of a game object
	class GENGINE_API Attribute : public Component
	{
	public:
		Attribute() = default;
		Attribute(const Attribute&) = default;

		UUID GetUUID() const { return m_UUID; }
	public:
		bool m_IsActive = true;
		// Name of the game object
		std::string m_Name = "";
	private:
		UUID m_UUID;
		friend class Scene;
	};
}


