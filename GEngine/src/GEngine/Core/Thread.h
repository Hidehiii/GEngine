#pragma once

#include "GEngine/Core/Core.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace GEngine
{
	class GENGINE_API Thread
	{
	public:
		static std::mutex s_Mutex;
	};

	template<typename T>
	class GENGINE_API TripleAtomicBuffer
	{
	public:
		TripleAtomicBuffer(bool waitConsume)
		{
			m_Producer	= &m_Buffers[0];
			m_Swp		= &m_Buffers[1];
			m_Consumer	= &m_Buffers[2];

			m_ReadyForConsume = false;
			m_ReadyForProduce = true;

			m_WaitConsume = waitConsume;
		}

		~TripleAtomicBuffer()
		{
			m_Producer	= nullptr;
			m_Swp		= nullptr;
			m_Consumer	= nullptr;
		}

		bool Produce()
		{
			if (m_WaitConsume)
			{
				bool ready = true;
				if (m_ReadyForProduce.compare_exchange_weak(ready, false) == false)
					return false;
			}
			T* curSwp = m_Swp;
			while (m_Swp.compare_exchange_weak(curSwp, m_Producer) == false)
			{
				curSwp = m_Swp;
			}
			m_Producer = curSwp;

			m_ReadyForConsume = true;
			return true;
		}

		bool Consume()
		{
			bool ready = true;
			if (m_ReadyForConsume.compare_exchange_weak(ready, false) == false)
				return false;

			T* curSwp = m_Swp;
			while (m_Swp.compare_exchange_weak(curSwp, m_Consumer) == false)
			{
				curSwp = m_Swp;
			}
			m_Consumer = curSwp;
			
			m_ReadyForProduce = true;
			return true;
		}

		T* GetProducer()
		{
			return m_Producer;
		}

		T* GetConsumer()
		{
			return m_Consumer;
		}
	private:
		T	m_Buffers[3];
		T*	m_Producer = nullptr;
		T*	m_Consumer = nullptr;

		std::atomic<T*>	m_Swp;

		std::atomic<bool> m_ReadyForConsume;
		std::atomic<bool> m_ReadyForProduce;

		bool m_WaitConsume;
	};
}

#define MAIN_THREAD_MUTEX_LOCK std::lock_guard<std::mutex> lock(Thread::s_Mutex)
#define THREAD_ID std::this_thread::get_id()
