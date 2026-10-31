#pragma once

#include <GEngine/Scripting/NativeScriptBehaviour.h>

namespace GameWorld
{
	class NativeCounterScript final : public GEngine::NativeScriptBehaviour
	{
	public:
		void OnAwake() override;
		void OnStart() override;
		void OnFixedUpdate(float fixedDeltaTime) override;
		void OnUpdate(float deltaTime) override;
		void OnLateUpdate() override;
		void OnPause() override;
		void OnResume() override;
		void OnDestroy() override;

	private:
		int m_FixedUpdates = 0;
		int m_Updates = 0;
	};
}
