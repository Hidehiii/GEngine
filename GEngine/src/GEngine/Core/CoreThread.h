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



