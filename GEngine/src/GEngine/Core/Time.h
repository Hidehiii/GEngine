#pragma once

#include "GEngine/Core/Core.h"

namespace GEngine
{
	class GENGINE_API Time
	{
	public:
		Time() = default;

		static float GetDeltaTime() { return m_DeltaTime; }
		static void SetDeltaTime(float deltaTime) { m_DeltaTime = deltaTime; }

		static float GetFixedTime() { return m_FixedTime; }
		static void SetFixedTime(float fixedTime) { m_FixedTime = fixedTime; }

		static float GetRunTime() { return m_RunTime; }
		static void SetRunTime(float runTime) { m_RunTime = runTime; }
	public:
		static float m_DeltaTime;
		static float m_FixedTime;
		static float m_RunTime;
	};

template<typename T>
class Timer
{
public:
	Timer(const char* name, T&& func)
		: m_Name(name), m_Func(func), m_Stopped(false)
	{
		m_StartTimePoint = std::chrono::high_resolution_clock::now();
	}

	~Timer()
	{
		if (!m_Stopped)
		{
			Stop();
		}
	}

	void Stop()
	{
		auto endTimePoint = std::chrono::high_resolution_clock::now();

		long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimePoint).time_since_epoch().count();
		long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch().count();

		m_Stopped = true;

		float duration = (end - start) * 0.001f;

		m_Func({ m_Name, duration });
	}

private:
	const char* m_Name;
	bool m_Stopped;
	T m_Func;
	std::chrono::time_point<std::chrono::steady_clock> m_StartTimePoint;
};
}



