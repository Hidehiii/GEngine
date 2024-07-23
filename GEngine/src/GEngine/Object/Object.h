#pragma once
#include "GEngine/Core/Core.h"
#include <entt.hpp>

namespace GEngine
{
	class GENGINE_API Object
	{
	public:
		Object();
		~Object() = default;

		operator bool() const { return m_EntityHandle != entt::null; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }
		operator uintptr_t() const { return (uintptr_t)(uint32_t)m_EntityHandle; }
		operator entt::entity() const { return m_EntityHandle; }
	protected:
		entt::entity m_EntityHandle{ entt::null };
	};
}



