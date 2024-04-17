#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Core/UUID.h"
#include "GEngine/Components/Component.h"
namespace GEngine
{
	class GENGINE_API ID : public Component
	{
	public:
		ID();
		ID(uint64_t uuid);
		ID(const ID&) = default;


		UUID GetUUID() const { return m_UUID; }
		operator uint64_t() const { return (uint64_t)m_UUID; }

	private:
		UUID m_UUID;
	};
}


