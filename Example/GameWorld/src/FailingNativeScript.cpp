#include "FailingNativeScript.h"

#include <GEngine/Core/Log.h>

#include <stdexcept>

namespace GameWorld
{
	GE_REGISTER_NATIVE_SCRIPT(FailingNativeScript, "GameWorld.FailingNativeScript")

	void FailingNativeScript::OnStart()
	{
		GE_INFO("[FailingNativeScript] Start");
	}

	void FailingNativeScript::OnUpdate(float)
	{
		throw std::runtime_error("Expected native script failure.");
	}

	void FailingNativeScript::OnDestroy()
	{
		GE_INFO("[FailingNativeScript] Destroy");
	}
}
