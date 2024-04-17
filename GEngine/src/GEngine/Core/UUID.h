#pragma once

#include "GEngine/Core/Core.h"

#include <xhash>
namespace GEngine
{
	class GENGINE_API UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		operator uint64_t() const { return m_UUID; }
	private:
		uint64_t m_UUID;
	};
}

namespace std
{
	template<>
	struct hash<GEngine::UUID>
	{
		std::size_t operator()(const GEngine::UUID& uuid) const
		{
			return std::hash<uint64_t>()((uint64_t)uuid);
		}
	};
}
