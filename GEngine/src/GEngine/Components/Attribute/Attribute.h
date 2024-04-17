#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Components/Component.h"

namespace GEngine
{
	class GENGINE_API Attribute : public Component
	{
	public:
		Attribute() = default;
		Attribute(const Attribute&) = default;
	public:
		bool m_IsActive = true;
	};
}


