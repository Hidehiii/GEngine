#pragma once

#include <GEngine/Scripting/NativeScriptBehaviour.h>

namespace GameWorld
{
	class FailingNativeScript final : public GEngine::NativeScriptBehaviour
	{
	public:
		void OnStart() override;
		void OnUpdate(float deltaTime) override;
		void OnDestroy() override;
	};
}
