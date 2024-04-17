#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Components/Component.h"
#include <string>

namespace GEngine
{
	class GENGINE_API Name : public Component
	{
	public:
		Name(std::string name = "")
			: m_Name(name) {}
		Name(const Name&) = default;
		std::string m_Name;
	};
}


