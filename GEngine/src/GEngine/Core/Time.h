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

    class GENGINE_API TimerWheel {
    public:
        using Task = std::function<void()>;

        explicit TimerWheel(size_t wheel_size, int interval_ms)
            : wheel_size_(wheel_size),
            interval_ms_(interval_ms),
            wheel_(wheel_size),
            current_index_(0) {}

        ~TimerWheel() {
            Stop();
        }

        void Start() {
            if (running_) {
                return;
            }
            running_ = true;
            thread_ = std::thread([this]() {
                while (running_) {
                    if (pausing_)
                    {
                        continue;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms_));
                    Tick();
                }
                GE_CORE_INFO("timer oooops!");
                });
            thread_.detach();
        }

        void Pause()
        {
            if (!running_)
            {
                return;
            }
            pausing_ = true;
        }

        void Continue()
        {
            if (!running_)
            {
                return;
            }
            pausing_ = false;
        }

        void Stop() {
            if (!running_) {
                return;
            }
            running_ = false;
            if (thread_.joinable()) {
                thread_.join();
            }
        }

        void AddTask(int timeout_ms, Task task) {
            std::lock_guard<std::mutex> lock(CoreThread::s_Mutex);
            size_t ticks = timeout_ms / interval_ms_;
            size_t index = (current_index_ + ticks) % wheel_size_;
            size_t allindex = index;
            for (size_t i = 1; allindex < wheel_size_; i++)
            {
                allindex = index * i;
                if (allindex >= wheel_size_)
                    break;
                wheel_[allindex].push_back(task);
            }

        }

    private:
        void Tick() {
            std::lock_guard<std::mutex> lock(CoreThread::s_Mutex);
            if(running_ == false || pausing_ == true)
			{
				return;
			}
            auto& tasks = wheel_[current_index_];
            for (const auto& task : tasks) {
                task();
            }
            current_index_ = (current_index_ + 1) % wheel_size_;
        }

    private:
        size_t wheel_size_;
        int interval_ms_;
        std::vector<std::list<Task>> wheel_;
        size_t current_index_;
        static bool running_;
        static bool pausing_;
        std::thread thread_;
    };


}



