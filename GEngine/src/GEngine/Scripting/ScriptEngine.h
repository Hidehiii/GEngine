#pragma once

#include "GEngine/Core/Core.h"

namespace GEngine
{
	class GENGINE_API ScriptEngine
	{
	public:

		static void Init();
		static void Shutdown();
	private:
		static void InitMono();
		static void ShutdownMono();
	};
}


