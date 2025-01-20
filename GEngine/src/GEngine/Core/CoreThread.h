#pragma once

#include "GEngine/Core/Core.h"

#include <thread>
#include <mutex>

namespace GEngine
{
	class GENGINE_API CoreThread
	{
	public:
		static std::mutex s_Mutex;
	};
}

#define CORE_THREAD_MUTEX CoreThread::s_Mutex
#define THREAD_ID std::this_thread::get_id()
