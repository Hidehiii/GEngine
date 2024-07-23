#pragma once
#include "GEngine/Core/Core.h"

#include <string>

namespace GEngine
{
	class GENGINE_API FileDialogs
	{
	public:
		// Return empty string if cancelled
		static std::string OpenFile(const char* filter);
		static std::string SaveFile(const char* filter);
	};
}