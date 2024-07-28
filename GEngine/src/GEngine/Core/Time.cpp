#include "GEpch.h"
#include "Time.h"

namespace GEngine
{
	float Time::s_DeltaTime = 0.0f;
	float Time::s_FixedTime = 0.02f;
	float Time::s_RunTime = 0.0f;
	float Time::s_PhysicsDeltaTime = 0.0f;

	bool PhysicsTimerWheel::s_Running = false;
	bool PhysicsTimerWheel::s_Pausing = false;
}