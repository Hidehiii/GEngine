#pragma once

#include "GEngine/Core/Core.h"

namespace GEngine
{
	struct RuntimeSpecification;

	class GENGINE_API ScriptEngine
	{
	public:

		static bool Init(const RuntimeSpecification& specification);
		static void Shutdown();
	private:
		static bool InitMono(const RuntimeSpecification& specification);
		static void ShutdownMono();
	};
}


