#include "NativeCounterScript.h"

#include <GEngine/Core/Log.h>

namespace GameWorld
{
	GE_REGISTER_NATIVE_SCRIPT(NativeCounterScript, "GameWorld.NativeCounterScript")

	void NativeCounterScript::OnAwake()
	{
		GE_INFO("[NativeScript] Awake {0}:{1}:{2}",
			static_cast<uint64_t>(GetGameObject().GetUUID()),
			GetGameObject().GetName(),
			GetGameObject().IsActive());
	}

	void NativeCounterScript::OnStart()
	{
		GE_INFO("[NativeScript] Start {0}:{1}",
			static_cast<uint64_t>(GetGameObject().GetUUID()),
			GetGameObject().GetName());
	}

	void NativeCounterScript::OnFixedUpdate(float fixedDeltaTime)
	{
		++m_FixedUpdates;
		if (m_FixedUpdates == 1)
			GE_INFO("[NativeScript] Fixed {0}", fixedDeltaTime);
	}

	void NativeCounterScript::OnUpdate(float deltaTime)
	{
		++m_Updates;
		if (m_Updates == 1)
			GE_INFO("[NativeScript] Update {0}", deltaTime);
	}

	void NativeCounterScript::OnLateUpdate()
	{
		if (m_Updates == 1)
			GE_INFO("[NativeScript] LateUpdate");
	}

	void NativeCounterScript::OnPause()
	{
		GE_INFO("[NativeScript] Pause");
	}

	void NativeCounterScript::OnResume()
	{
		GE_INFO("[NativeScript] Resume");
	}

	void NativeCounterScript::OnDestroy()
	{
		GE_INFO("[NativeScript] Destroy fixed={0} update={1}", m_FixedUpdates, m_Updates);
	}
}
