#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Core/CoreThread.h"

#include <chrono>
#include <functional>
#include <list>
#include <vector>

namespace GEngine
{
	class GENGINE_API Time
	{
	public:
		Time() = default;

		static float GetDeltaTime() { return s_DeltaTime; }
		static void SetDeltaTime(float deltaTime) { s_DeltaTime = deltaTime; }

		static float GetFixedTime() { return s_FixedTime; }
		

		static float GetRunTime() { return s_RunTime; }
		static void SetRunTime(float runTime) { s_RunTime = runTime; }

        static float GetPhysicsDeltaTime() { return s_PhysicsDeltaTime; }
        static void SetPhysicsDeltaTime(float physicsDeltaTime) { s_PhysicsDeltaTime = physicsDeltaTime; }

    private:
        static void SetFixedTime(float fixedTime) { s_FixedTime = fixedTime; }

	public:
		static float s_DeltaTime;
		static float s_RunTime;
        static float s_PhysicsDeltaTime;
    private:         
        static float s_FixedTime;
	};

	template<typename T>
	class GENGINE_API Timer
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

    class GENGINE_API PhysicsTimerWheel {
    public:
        using Task = std::function<void()>;

        explicit PhysicsTimerWheel(size_t wheelSize, int interval_ms)
            : m_WheelSize(wheelSize),
            m_Interval_ms(interval_ms),
            m_Wheel(wheelSize),
            m_CurrentIndex(0) {}

        ~PhysicsTimerWheel() {
            Stop();
        }

        void Start() {
            if (s_Running) {
                return;
            }
            s_Running = true;
            s_Pausing = false;
            m_Thread = std::thread([this]() {
                while (s_Running) {
                    if (s_Pausing)
                    {
                        continue;
                    }
                    auto startTimePoint = std::chrono::high_resolution_clock::now();
                    std::this_thread::sleep_for(std::chrono::microseconds(m_Interval_ms));
                   
                    Tick();
                    auto endTimePoint = std::chrono::high_resolution_clock::now();

                    long long start = std::chrono::time_point_cast<std::chrono::microseconds>(startTimePoint).time_since_epoch().count();
                    long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch().count();

                    float duration = (end - start) * 0.001f * 0.001f;
                    Time::SetPhysicsDeltaTime(duration);
                }
                });
            m_Thread.detach();
        }

        void Pause()
        {
            if (!s_Running)
            {
                return;
            }
            s_Pausing = true;
        }

        void Continue()
        {
            if (!s_Running)
            {
                return;
            }
            s_Pausing = false;
        }

        void Stop() {
            if (!s_Running) {
                return;
            }
            s_Running = false;
            s_Pausing = false;
            if (m_Thread.joinable()) {
                m_Thread.join();
            }
        }

        void AddTask(int timeout_ms, Task task) {
            std::lock_guard<std::mutex> lock(CoreThread::s_Mutex);
            GE_CORE_ASSERT(m_WheelSize * m_Interval_ms >= timeout_ms, "Out of range");
            size_t ticks = timeout_ms / m_Interval_ms;
            size_t index = (m_CurrentIndex + ticks) % m_WheelSize;
            size_t allindex = index;
            for (size_t i = 0; allindex < m_WheelSize; i++)
            {
                allindex = index * i;
                if (allindex >= m_WheelSize)
                    break;
                m_Wheel[allindex].push_back(task);
            }

        }

    private:
        void Tick() {
            std::lock_guard<std::mutex> lock(CoreThread::s_Mutex);
            if(s_Running == false || s_Pausing == true)
			{
				return;
			}
            auto& tasks = m_Wheel[m_CurrentIndex];
            for (const auto& task : tasks) {
                task();
            }
            m_CurrentIndex = (m_CurrentIndex + 1) % m_WheelSize;
        }

    private:
        size_t m_WheelSize;
        int m_Interval_ms;
        std::vector<std::list<Task>> m_Wheel;
        size_t m_CurrentIndex;
        static bool s_Running;
        static bool s_Pausing;
        std::thread m_Thread;
    };


}



