#include "GEpch.h"
#include "ID.h"

GEngine::ID::ID()
{
	m_UUID = UUID();
}

GEngine::ID::ID(uint64_t uuid)
{
	m_UUID = UUID(uuid);
}
